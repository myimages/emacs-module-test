;; This buffer is for text that is not saved, and for Lisp evaluation.
;; To create a file, visit it with <open> and enter text in its buffer.

(require 'ert)
(require 'module-test)
(require 'cl)

;; (&optional sources input prompt resume preselect
;;             buffer keymap default history allow-nest).

(ert-deftest test-candidate-list ()
  (let ((score (calc-score "foo" "foobar")))
    (should (equal score 0.75))))

(message "Helm find files")
(benchmark 100000 '(helm-ff-score-candidate-for-pattern "score the needle. in the haystack" "needle."))

(message "Helm regular socring")
(benchmark 100000 '(helm-score-candidate-for-pattern "score the needle in the haystack" "needle"))

(message "command T scorer")
(benchmark 100000 '(calc-score "needle" "score the needle in the haystack"))

(message "Git-find-files scorer")
(benchmark 100000 '(gff-score-fuzzily "needle" "score the needle in the haystack"))
