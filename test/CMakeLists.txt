set(input_path ${CMAKE_SOURCE_DIR}/test/input/)
set(output_path ${CMAKE_SOURCE_DIR}/test/output/)
set(diagram_path ${CMAKE_SOURCE_DIR}/test/diagram/)

function (test num)
add_test(
    NAME test_ROBDD_${num}
    COMMAND ROBDD ${num} ${input_path} ${output_path} ${diagram_path}
)
endfunction()
foreach(num RANGE 1 12)
 test(${num})
endforeach()

