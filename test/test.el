;;; test.el --- dynamic module test

;; Copyright (C) 2015 by Syohei YOSHIDA

;; Author: Syohei YOSHIDA <syohex@gmail.com>

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Code:

(require 'ert)
(require 'module-test)


(ert-deftest test-calc-score ()
  (should (= (perfect-score "foo" "foobar") 0.85)))



(ert-deftest test-consecutive-match ()
  "leading word separators should be penalized"
  (let* ((query "ido")
         (higher (perfect-score query "lisp/ido.el"))
         (lower (perfect-score query "lisp/dom.el")))
    (should (> higher lower))))
