(require 'ert)
(require 'module-test)


(ert-deftest test-calc-score ()
    (should (= (calc-score "needle" "score the needle in the haystack") 0.5739583373069763)))

;; (ert-deftest foo ()
;;     (should (= (calc-score "Foo" "Foobar" t) 0.75)))
