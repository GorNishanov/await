<style type="text/css">
    p {text-align:justify}
    li {text-align:justify}
    blockquote.note
    {
      background-color:#E0E0E0;
      padding-left: 15px;
      padding-right: 15px;
      padding-top: 1px;
      padding-bottom: 1px;
    }
    ins {background-color:#A0FFA0}
    del {background-color:#FFA0A0}
    table {border-collapse: collapse;}
    table, th, td {
      border: 1px solid black;
      border-collapse: collapse;
    }
  </style>


| Document Number: | D0912R5                                         |
| -----------------|-------------------------------------------------|
| Date:            | 2019-02-22                                      |
| Audience:        | WG21                                            |
| Revises:         | P0912R4                                         |
| Reply to:        | gorn@microsoft.com                              |


Merge Coroutines TS into C++20 working draft
============================================

## Abstract

This paper proposes merging Working Draft of Coroutines TS [N4775] into the C++20 working draft [N4800].

## Revision history

r0: initial revision

r1:
* Markdown rendered as HTML.
* Replaced ligature ff with two letters ff.
* Expanded on compiler availability.
* Reworded the closing paragraph.
* Make insertions <ins>green</ins>.
* Updated working draft numbers

r2:
* ... keywords s/were/are added ...
* P0664R3 => P0664R4
* Added list of edits requested by LWG on 2018-06-08.

r3:
* document numbers and edition instructions updated

r4:
* document numbers and edition instructions updated

r5:
* incorporated the resolutions to Coroutines Issues #25 #27 (https://wg21.link/P1356R0)
* incorporated the resolution to Coroutines Issues #31 #35 (https://wg21.link/P0664R7)

## Introduction

* Coroutines of this kind were available in a shipping compiler since 2014.
* We have been receiving and acting upon developer feedback for 5 years.
* The software built using coroutines has been deployed on more than **400 million** devices in the hands of delighted consumers.
* The software built using coroutines on Linux and Windows is powering the foundational services of Windows Azure cloud services.
* Coroutines have been used by thousands of software developers in various companies.
* We have two publicly available implementations of coroutines TS in MSVC since version 2015 SP2 (2016) and clang version 5 (2017).
* We have an independent coroutine implementation in EDG frontend (2015).
* GCC implementation is in progress.
* There are several open source coroutine abstraction libraries, including an extensive open source coroutine library <a href="https://github.com/lewissbaker/cppcoro">cppcoro</a> that utilizes most of the coroutine features with tests that can be used to smoke test emerging coroutine implementation for completeness and correctness.
* Other libraries provide coroutines TS bindings for its types, for example, Facebook's Folly, Just::Thread Pro and others.

Coroutines address the dire need by dramatically simplifying development of asynchronous code.
Coroutines have been available and in use for 5 years. We have shipping implementations from two major
compiler vendors. It is time to merge Coroutines TS to the working paper.
 <!-- to unblock development related libraries utilizing coroutine language facilities. -->

## Wording
- Apply coroutine wording from N4775 to the working draft with the following changes:
    - replace `experimental::` with nothing
    - replace `<experimental/coroutine>` with `<coroutine>`
    - in synopsis in [coroutine.trivial.awaitables] and [support.coroutine]
      - remove "namespace experimental {" and "} // namespace experimental"
      - remove "inline namespace coroutines_v1 {" and "} // namespace coroutines_v1"
    - exclude changes to [stmt.iter] and [stmt.ranged] (issue #35 incorporated)
    - LaTex specific instructions: before applying the changes, in the coroutine TS LaTeX source, replace all occurrences of \cxxref{<i>stable-name</i>} with \ref{<i>stable-name</i>}.

- Add underlined text to rationale in [diff.cpp17.lex]:

> **Rationale:** Required for new features. The `requires` keyword is added to introduce constraints through a _requires-clause_ or a _requires-expression_. The `concept` keyword is added to enable the definition of concepts (12.6.8). <ins>The `co_await`, `co_yield`, and `co_return` keywords are added to enable the definition of coroutines (\ref{dcl.fct.def.coroutine}).</ins> Effect on original feature: Valid ISO C++ 2017 code using `concept`<ins>, `co_await`, `co_yield`, `co_return`,</ins> or `requires` as an identifier is not valid in this International Standard.

- Add underlined text to "Effect on original feature" in [diff.cpp17.library]:

> **Effect on original feature:** The following C++ headers are new: `<compare>`<ins>, `<coroutine>`,</ins> and `<syncstream>`. Valid C++ 2017 code that #includes headers with these names may be invalid in this International Standard.
- Apply the following modifications that implement the resolutions of issues 25, 27, and 31
- modify [expr.await]/2 as follows:

> An _await-expression_ shall appear only in a potentially-evaluated expression within the _compound-statement_ of a _function-body_ outside of a _handler_ (Clause 18). In a declaration-statement or in the simple-declaration (if any) of a for-init-statement, an <del>await-expression</del><ins>_await-expression_</ins> shall appear only in an initializer of that declaration-statement or simple-declaration. <ins>An _await-expression_ shall not appear in the initializer of a block-scope variable with static or thread storage duration.</ins>

- in section [dcl.fct.def.coroutine] add two paragraphs after paragraph 11:

> <ins>12. If the evaluation of the expression _p_`.unhandled_exception()` exits via an exception, the coroutine is considered suspended at the final suspend point.</ins>

> <ins>13. The expression `co_await` _p_`.final_suspend()` shall not be potentially-throwing ([except.spec]).</ins>

- add the underlined text at the beginning of [coroutine.handle.resumption]:

> <ins>1. Resuming a coroutine via `resume`, `operator()`, or `destroy` on an execution agent other than the one it was suspended on has implementation-defined behavior unless each is either an instance of `std::thread` or the thread that executes `main`.</ins>

> <ins><i>[Note:</i> A coroutine that is resumed on a different execution agent should avoid relying on consistent thread identity throughout, such as holding a mutex object across a suspend point. — <i>end note ]</i></ins>

> <ins><i>[Note:</i> A concurrent resumption of the coroutine may result in a data race. — <i>end note ]</i></ins>

- remove all occurrences of this note from [coroutine.handle.resumption]:

> <del><i>[ Note:</i> A concurrent resumption of the coroutine via resume, operator(), or destroy may result in a data race. <i>—end note ]</i></del>


