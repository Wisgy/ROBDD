set(input_path ${CMAKE_SOURCE_DIR}/test/input/)
set(output_path ${CMAKE_SOURCE_DIR}/test/output/)

function (test num)
add_test(
    NAME test_ROBDD_${num}
    COMMAND ROBDD ${num} ${input_path} ${output_path}
)
endfunction()
foreach(num RANGE 0 9)
 test(${num})
endforeach()

