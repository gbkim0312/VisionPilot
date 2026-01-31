# 보류 대상:
# 항목명: cert-err58-cpp   | 효과: 초기화시 예외를 던질 수 있는 객체가 전역이나 static 또는 thread_local로 선언되어 있으면 잡는다.  | 보류 사유 :  v2x_util::AsnConst의 static 변수들을 사용하는 함수가 지나치게 많고, 일부 GTEST 관련 변수까지 잡게 됨.
# 항목명: cppcoreguidelines-pro-bounds-constant-array-index   | 효과: 정적 배열이나 std::array가 상수 정수 index를 가지지 않거나 index가 범위가 벗어나는 것을 잡는다. | 보류 사유 : 정수 리터럴이 아닌 모든 index를 잡음(const int 변수 포함)
# 항목명: cppcoreguidelines-pro-type-const-cast   | 효과: const_cast를 사용하면 잡는다.  | 보류 사유 : 현재 사용하는 C-style cast 중 const_cast를 써야 하는 부분이 있음
# 항목명: readability-magic-numbers   | 효과: 매직 넘버 사용시 잡는다.  | 보류 사유 : AutoSar 규정에 포함되나, 분량이 너무 많아 우선 생략함
# 항목명: cppcoreguidelines-avoid-goto | 효과: goto 문 사용 시 잡는다. | 보류 사유 : MISRA C++ 2008 기준, Rule 6–6–1 에 의해 예외적 허용이 있으므로 생략함
set(V2X2_CLANG_TIDY_FLAGS
"bugprone-*,\
-bugprone-narrowing-conversions,\
-bugprone-lambda-function-name,\
-bugprone-easily-swappable-parameters,\
modernize-*,\
-modernize-use-trailing-return-type,\
-modernize-use-emplace,\
-modernize-use-nodiscard,\
performance-*,\
clang-diagnostic-*,\
clang-analyzer-*,\
-clang-analyzer-core.CallAndMessage,\
cppcoreguidelines-*,\
-cppcoreguidelines-pro-type-union-access,\
-cppcoreguidelines-avoid-c-arrays,\
-cppcoreguidelines-avoid-goto,\
-cppcoreguidelines-avoid-magic-numbers,\
-cppcoreguidelines-c-copy-assignment-signature,\
-cppcoreguidelines-explicit-virtual-functions,\
-cppcoreguidelines-macro-usage,\
-cppcoreguidelines-non-private-member-variables-in-classes,\
-cppcoreguidelines-owning-memory,\
-cppcoreguidelines-pro-bounds-constant-array-index,\
-cppcoreguidelines-pro-type-const-cast,\
-cppcoreguidelines-pro-type-reinterpret-cast,\
-cppcoreguidelines-pro-type-static-cast-downcast,\
-cppcoreguidelines-avoid-do-while,\
-clang-diagnostic-deprecated-declarations,\
-cppcoreguidelines-avoid-non-const-global-variables,\
misc-*,\
-misc-header-include-cycle,\
-misc-const-correctness,\
-misc-include-cleaner,\
-misc-non-copyable-objects,\
-misc-static-assert,\
-misc-unused-using-decls,\
cert-dcl50-cpp,\
cert-dcl58-cpp,\
cert-err34-c,\
cert-err52-cpp,\
cert-err60-cpp,\
cert-msc50-cpp,\
cert-msc51-cpp,\
cert-oop58-cpp,\
fuchsia-multiple-inheritance,\
fuchsia-virtual-inheritance,\
google-build-namespaces,\
google-build-using-namespace,\
google-explicit-constructor,\
google-global-names-in-headers,\
google-readability-casting,\
google-runtime-int,\
google-runtime-operator,\
hicpp-exception-baseclass,\
hicpp-multiway-paths-covered,\
hicpp-no-assembler,\
hicpp-signed-bitwise,\
readability-avoid-const-params-in-decls,\
readability-braces-around-statements,\
readability-const-return-type,\
readability-container-size-empty,\
readability-delete-null-pointer,\
readability-deleted-default,\
readability-implicit-bool-conversion,\
readability-misplaced-array-index,\
readability-inconsistent-declaration-parameter-name,\
readability-isolate-declaration,\
readability-make-member-function-const,\
readability-qualified-auto,\
readability-redundant-access-specifiers,\
readability-redundant-declaration,\
readability-redundant-function-ptr-dereference,\
readability-redundant-preprocessor,\
readability-redundant-smartptr-get,\
readability-redundant-string-cstr,\
readability-redundant-string-init,\
readability-simplify-boolean-expr,\
readability-simplify-subscript-expr,\
readability-uppercase-literal-suffix")

function(set_global_clang_flags targetname)
  if(${ACV2X_TOOLCHAIN_NAME} STREQUAL "x86-64")
    set_target_properties(
      ${targetname}
      PROPERTIES
      CXX_CLANG_TIDY
"${CLANG_TIDY_EXE};-checks=-*,\
${V2X2_CLANG_TIDY_FLAGS};\
-warnings-as-errors=-*,\
${V2X2_CLANG_TIDY_FLAGS};\
--extra-arg=-std=c++17"
    )
  endif()
endfunction(set_global_clang_flags targetname)

function(set_no_clang_flags targetname)
  if(${ACV2X_TOOLCHAIN_NAME} STREQUAL "x86-64")
    set_target_properties(
      ${targetname}
      PROPERTIES
      CXX_CLANG_TIDY ""
    )
  endif()
endfunction(set_no_clang_flags)

function(set_gtest_clang_flags targetname)
  if(${ACV2X_TOOLCHAIN_NAME} STREQUAL "x86-64")
    set(LOCAL_CLANG_TIDY_FLAGS
"${V2X2_CLANG_TIDY_FLAGS},\
-cppcoreguidelines-special-member-functions,\
-cppcoreguidelines-avoid-goto,\
-cppcoreguidelines-pro-type-member-init,\
-misc-non-private-member-variables-in-classes")

    set_target_properties(
      ${targetname}
      PROPERTIES
      CXX_CLANG_TIDY
"${CLANG_TIDY_EXE};-checks=-*,\
${LOCAL_CLANG_TIDY_FLAGS};\
-warnings-as-errors=-*,\
${LOCAL_CLANG_TIDY_FLAGS};\
--extra-arg=-std=c++17"
    )
  endif()
endfunction(set_gtest_clang_flags)

function(set_tui_clang_flags targetname)
  if(${ACV2X_TOOLCHAIN_NAME} STREQUAL "x86-64")
    set(LOCAL_CLANG_TIDY_FLAGS
"${V2X2_CLANG_TIDY_FLAGS},\
-cppcoreguidelines-pro-type-vararg,\
-cppcoreguidelines-pro-bounds-array-to-pointer-decay,\
-hicpp-signed-bitwise")

    set_target_properties(
      ${targetname}
      PROPERTIES
      CXX_CLANG_TIDY
"${CLANG_TIDY_EXE};-checks=-*,\
${LOCAL_CLANG_TIDY_FLAGS};\
-warnings-as-errors=-*,\
${LOCAL_CLANG_TIDY_FLAGS};\
--extra-arg=-std=c++17"
    )
  endif()
endfunction(set_tui_clang_flags)

function(get_all_targets _result _dir)
  get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)

  foreach(_subdir IN LISTS _subdirs)
    get_all_targets(${_result} "${_subdir}")
  endforeach()

  get_property(_sub_targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
  set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

function(add_subdirectory_with_static_analysis _folder)
  add_subdirectory(${_folder} ${ARGN})
  get_all_targets(_targets "${_folder}")

    foreach(_target IN LISTS _targets)
      if (_target MATCHES "^.*_test")
        set_gtest_clang_flags("${_target}")
        message(STATUS "Setting clang-tidy properties of target: ${_target} - Gtest Flags")
      elseif ("${_target}" STREQUAL "certTool_dev")
        set_tui_clang_flags("${_target}")
        message(STATUS "Setting clang-tidy properties of target: ${_target} - TUI Flags")
      elseif (
        ("${_target}" STREQUAL "asn") OR
        ("${_target}" MATCHES "asn1c") OR
        ("${_target}" STREQUAL "coverage") OR
        ("${_target}" STREQUAL "cpp-terminal") OR
        ("${_target}" STREQUAL "spdlog") OR
        ("${_target}" STREQUAL "leveldb") OR
        ("${_target}" STREQUAL "cryptofunction")
        )
        message(STATUS "Setting clang-tidy properties of target: ${_target} - Omit")
      else()
        set_global_clang_flags("${_target}")
        message(STATUS "Setting clang-tidy properties of target: ${_target} - Global Flags")
      endif()
    endforeach()
endfunction()
