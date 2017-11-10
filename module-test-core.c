/*
  Copyright (C) 2015 by Syohei YOSHIDA

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h> /* for FLT_MAX */
#include <string.h>


#include "emacs-module.h"

#define UNSET_SCORE FLT_MAX

int plugin_is_GPL_compatible;

// Use a struct to make passing params during recursion easier.
typedef struct {
        const char  *haystack_p;        // Pointer to the path string to be searched.
        long        haystack_len;       // Length of same.
        const char  *needle_p;          // Pointer to search string (needle).
        long        needle_len;         // Length of same.
        long        *rightmost_match_p; // Rightmost match for each char in needle.
        float       max_score_per_char;
        int         case_sensitive;     // Boolean.
        float       *memo;              // Memoization.
} matchinfo_t;

float recursive_match(
                      matchinfo_t *m,    // Sharable meta-data.
                      long haystack_idx, // Where in the path string to start.
                      long needle_idx,   // Where in the needle string to start.
                      long last_idx,     // Location of last matched character.
                      float score        // Cumulative score so far.
                      ) {
        long distance, i, j;
        float *memoized = NULL;
        float score_for_char;
        float seen_score = 0;

        // Iterate over needle.
        for (i = needle_idx; i < m->needle_len; i++) {
                // Iterate over (valid range of) haystack.
                for (j = haystack_idx; j <= m->rightmost_match_p[i]; j++) {
                        char c, d;

                        // Do we have a memoized result we can return?
                        memoized = &m->memo[j * m->needle_len + i];
                        if (*memoized != UNSET_SCORE) {
                                return *memoized > seen_score ? *memoized : seen_score;
                        }
                        c = m->needle_p[i];
                        d = m->haystack_p[j];
                        if (d >= 'A' && d <= 'Z' && !m->case_sensitive) {
                                d += 'a' - 'A'; // Add 32 to downcase.
                        }

                        if (c == d) {
                                // Calculate score.
                                float sub_score = 0;
                                score_for_char = m->max_score_per_char;
                                distance = j - last_idx;

                                if (distance > 1) {
                                        float factor = 1.0;
                                        char last = m->haystack_p[j - 1];
                                        char curr = m->haystack_p[j]; // Case matters, so get again.
                                        if (last == '/') {
                                                factor = 0.9;
                                        } else if (
                                                   last == '-' ||
                                                   last == '_' ||
                                                   last == ' ' ||
                                                   (last >= '0' && last <= '9')
                                                   ) {
                                                factor = 0.8;
                                        } else if (
                                                   last >= 'a' && last <= 'z' &&
                                                   curr >= 'A' && curr <= 'Z'
                                                   ) {
                                                factor = 0.8;
                                        } else if (last == '.') {
                                                factor = 0.7;
                                        } else {
                                                // If no "special" chars behind char, factor diminishes
                                                // as distance from last matched char increases.
                                                factor = (1.0 / distance) * 0.75;
                                        }
                                        score_for_char *= factor;
                                }

                                if (j < m->rightmost_match_p[i]) {
                                        sub_score = recursive_match(m, j + 1, i, last_idx, score);
                                        if (sub_score > seen_score) {
                                                seen_score = sub_score;
                                        }
                                }
                                last_idx = j;
                                haystack_idx = last_idx + 1;
                                score += score_for_char;
                                *memoized = seen_score > score ? seen_score : score;
                                if (i == m->needle_len - 1) {
                                        // Whole string matched.
                                        return *memoized;
                                }
                        }
                }
        }
        return *memoized = score;
}

float calculate_match(
                      const char *haystack,
                      const char *needle,
                      int case_sensitive
                      ) {
        matchinfo_t m;
        long i;
        float score             = 1.0;
        m.haystack_p            = haystack;
        m.haystack_len          = strlen(haystack);
        m.needle_p              = needle;
        m.needle_len            = strlen(needle);
        m.rightmost_match_p     = NULL;
        m.max_score_per_char    = (1.0 / m.haystack_len + 1.0 / m.needle_len) / 2;
        m.case_sensitive        = case_sensitive;

        // Special case for zero-length search string.
        if (m.needle_len == 0) {
                return score;
        }

        long haystack_limit;
        long memo_size;
        long needle_idx;
        long mask;
        long rightmost_match_p[m.needle_len];

        // Pre-scan string:
        // - Bail if it can't match at all.
        // - Record rightmost match for each character (prune search space).
        m.rightmost_match_p = rightmost_match_p;
        needle_idx = m.needle_len - 1;
        mask = 0;
        for (i = m.haystack_len - 1; i >= 0; i--) {
                char c = m.haystack_p[i];
                char lower = c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
                if (!m.case_sensitive) {
                        c = lower;
                }
                if (needle_idx >= 0) {
                        char d = m.needle_p[needle_idx];
                        if (c == d) {
                                rightmost_match_p[needle_idx] = i;
                                needle_idx--;
                        }
                }
        }
        if (needle_idx != -1) {
                return 0.0;
        }

        // Prepare for memoization.
        haystack_limit = rightmost_match_p[m.needle_len - 1] + 1;
        memo_size = m.needle_len * haystack_limit;
        {
                float memo[memo_size];
                for (i = 0; i < memo_size; i++) {
                        memo[i] = UNSET_SCORE;
                }
                m.memo = memo;
                score = recursive_match(&m, 0, 0, 0, 0.0);
        }
        return score;
}


// command-t


static void
bind_function(emacs_env *env, const char *name, emacs_value Sfun)
{
        emacs_value Qfset = env->intern(env, "fset");
        emacs_value Qsym = env->intern(env, name);
        emacs_value args[] = { Qsym, Sfun };

        env->funcall(env, Qfset, 2, args);
}

static void
provide(emacs_env *env, const char *feature)
{
        emacs_value Qfeat = env->intern(env, feature);
        emacs_value Qprovide = env->intern (env, "provide");
        emacs_value args[] = { Qfeat };

        env->funcall(env, Qprovide, 1, args);
}




static emacs_value
Fcalc(emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
	emacs_value haystack = args[0];
	emacs_value needle = args[1];

	ptrdiff_t size = 0;
	ptrdiff_t size_2 = 0;
	env->copy_string_contents(env, haystack, NULL, &size);
	env->copy_string_contents(env, needle, NULL, &size_2);
	char *haystack_str = malloc(size);
	char *needle_str = malloc(size_2);
	env->copy_string_contents(env, haystack, haystack_str, &size);
	env->copy_string_contents(env, needle, needle_str, &size_2);

        float fuzzy_score = calculate_match(needle_str, haystack_str, 0);

	free(haystack_str);
	free(needle_str);

        return env->make_float(env,(double)fuzzy_score);
}

int
emacs_module_init(struct emacs_runtime *ert)
{
        emacs_env *env = ert->get_environment(ert);

#define DEFUN(lsym, csym, amin, amax, doc, data)                        \
        bind_function (env, lsym, env->make_function(env, amin, amax, csym, doc, data))

        DEFUN("calc-score", Fcalc, 2, 3, "Calculate match score", NULL);

#undef DEFUN

        provide(env, "module-test-core");
        return 0;
}

/*
  Local Variables:
  c-basic-offset: 8
  indent-tabs-mode: t
  End:
*/
