cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(PickerMatcher)

add_executable(
  pickermatcher
  src/main.cpp
)

target_compile_features(pickermatcher PUBLIC cxx_std_20)
target_compile_options(pickermatcher PUBLIC
  "/MP"          # Multi-threading support
  "/ZI"          # Edit and continue support
  "/permissive-"
  "/W4"          # Highest warning level
  "/w44062"   # enumerator 'identifier' in a switch of enum 'enumeration' is not handled
  "/w44255"   # 'function': no function prototype given: converting '()' to '(void)'
  "/w44289"   # nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
  "/w44296"   # 'operator': expression is always false
  "/w44311"   # 'variable' : pointer truncation from 'type' to 'type'
  "/w44339"   # 'type' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
  "/w44342"   # behavior change: 'function' called, but a member operator was called in previous versions
  "/w44350"   # behavior change: 'member1' called instead of 'member2'
  "/w44431"   # missing type specifier - int assumed. Note: C no longer supports default-int
  "/w44471"   # a forward declaration of an unscoped enumeration must have an underlying type (int assumed)
  "/w44545"   # expression before comma evaluates to a function which is missing an argument list
  "/w44546"   # function call before comma missing argument list
  "/w44547"   # 'operator': operator before comma has no effect; expected operator with side-effect
  "/w44548"   # expression before comma has no effect; expected expression with side-effect
  "/w44549"   # 'operator': operator before comma has no effect; did you intend 'operator'?
  "/w44555"   # expression has no effect; expected expression with side-effect
  "/w44574"   # 'identifier' is defined to be '0': did you mean to use '#if identifier'?
  "/w44608"   # 'symbol1' has already been initialized by another union member in the initializer list, 'symbol2'
  "/w44619"   # #pragma warning: there is no warning number 'number'
  "/w44628"   # digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'
  "/w44640"   # 'instance': construction of local static object is not thread-safe
  "/w44905"   # wide string literal cast to 'LPSTR'
  "/w44906"   # string literal cast to 'LPWSTR'
  "/w44946"   # reinterpret_cast used between related classes: 'class1' and 'class2'
  "/w44986"   # 'symbol': exception specification does not match previous declaration
  "/w44988"   # 'symbol': variable declared outside class/function scope
  "/wd4201"      # nonstandard extension used : nameless struct/union
)
target_include_directories(pickermatcher PUBLIC src)

add_subdirectory(ext)
target_link_libraries(pickermatcher PUBLIC fmt json)
