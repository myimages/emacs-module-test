(require 'ert)
(require 'module-test)


(ert-deftest test-calc-score ()
  "test score calculator"
  ;;573958
  (let ((score (calc-score "needle" "score the needle in the haystack")))
    (should (equal score 0.5739583373069763))))
