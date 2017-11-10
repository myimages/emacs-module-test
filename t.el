(require 'ert)
(require 'cl-lib)
(require 'module-test)


;; (ert-deftest test-calc-score ()
;;   "file open and close test"
;;   (let ((score (calc-score "needle" "score the needle in the haystack")))
;;     (should (= score 0.5739583373069763))))

(ert-deftest test-calc-score ()
  "test score calculator"
  ;;573958
  (let ((score (calc-score "needle" "score the needle in the haystack")))
    (should (equal score 0.5739583373069763))))
