project(nnacl)

set(NNACL_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${NNACL_DIR}/..)

if(ENABLE_CPU)
    set(CMAKE_C_FLAGS "-Wno-attributes ${CMAKE_C_FLAGS}")
endif()

if(APPLE OR PLATFORM_ARM32 OR PLATFORM_ARM64)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND DEFINED ARCHS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstrict-aliasing \
        -ffunction-sections -fdata-sections -ffast-math -Wno-shorten-64-to-32")
    endif()
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND NOT DEFINED ARCHS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fomit-frame-pointer -fstrict-aliasing \
        -ffunction-sections -fdata-sections -ffast-math")
    endif()
endif()

if(NOT MSVC)
    if("${X86_64_SIMD}" STREQUAL "avx")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1 -mavx -mavx2 -mfma")
    endif()
    if("${X86_64_SIMD}" STREQUAL "avx512")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1 -mavx -mavx2 -mfma -mavx512f")
    endif()
    if("${X86_64_SIMD}" STREQUAL "sse")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1")
    endif()
endif()

########################### files ###########################
file(GLOB KERNEL_SRC
    ${NNACL_DIR}/*.c
    ${NNACL_DIR}/fp32/*.c
    ${NNACL_DIR}/infer/*.c
    ${NNACL_DIR}/base/*.c
    ${NNACL_DIR}/fp32_grad/*.c
    ${NNACL_DIR}/kernel/*.c
    ${NNACL_DIR}/experimental/*.c
)

if(NOT MSLITE_ENABLE_RUNTIME_PASS)
  list(REMOVE_ITEM KERNEL_SRC ${NNACL_DIR}/infer/shape_fusion_infer.c)
endif()
if((NOT DEFINED MSLITE_ENABLE_INT8) OR MSLITE_ENABLE_INT8)
    file(GLOB KERNEL_SRC_INT8
            ${NNACL_DIR}/int8/*.c
            )
    set(KERNEL_SRC
            ${KERNEL_SRC}
            ${KERNEL_SRC_INT8}
            )
else()
    set(KERNEL_SRC
            ${KERNEL_SRC}
            ${NNACL_DIR}/int8/pack_int8.c
            ${NNACL_DIR}/int8/quantize.c
            )
endif()

if(MSLITE_ENABLE_SPARSE_COMPUTE)
    file(GLOB KERNEL_SRC_SPARSE
            ${NNACL_DIR}/fp32_sparse/*.c
            )
    set(KERNEL_SRC
            ${KERNEL_SRC}
            ${KERNEL_SRC_SPARSE}
            )
endif()

if(MSLITE_ENABLE_STRING_KERNEL)
    file(GLOB KERNEL_SRC_INFER_STRING
            ${NNACL_DIR}/infer/string/*.c
            )
    set(KERNEL_SRC
            ${KERNEL_SRC}
            ${KERNEL_SRC_INFER_STRING}
            )
endif()
if(MSLITE_ENABLE_CONTROLFLOW)
    file(GLOB KERNEL_SRC_INFER_CONTROL_TENSORLIST
            ${NNACL_DIR}/infer/control/*.c
            )
    set(KERNEL_SRC
            ${KERNEL_SRC}
            ${KERNEL_SRC_INFER_CONTROL_TENSORLIST}
            )
endif()
if(PLATFORM_ARM64)
    file(GLOB ASSEMBLY_SRC ${NNACL_DIR}/assembly/arm64/*.S)
    set_property(SOURCE ${ASSEMBLY_SRC} PROPERTY LANGUAGE C)
endif()

if(PLATFORM_ARM32)
    file(GLOB ASSEMBLY_SRC ${NNACL_DIR}/assembly/arm32/*.S)
    set_property(SOURCE ${ASSEMBLY_SRC} PROPERTY LANGUAGE C)
endif()

if("${X86_64_SIMD}" STREQUAL "sse")
    file(GLOB ASSEMBLY_SRC ${NNACL_DIR}/intrinsics/sse/*.c)
    set_property(SOURCE ${ASSEMBLY_SRC} PROPERTY LANGUAGE C)
endif()

if("${X86_64_SIMD}" STREQUAL "avx")
    file(GLOB ASSEMBLY_SRC ${NNACL_DIR}/intrinsics/sse/*.c
            ${NNACL_DIR}/intrinsics/avx/*.c
            ${NNACL_DIR}/assembly/avx/*.S
            ${NNACL_DIR}/intrinsics/ms_simd_cpu_info.c)
    set_property(SOURCE ${ASSEMBLY_SRC} PROPERTY LANGUAGE C)
endif()

if("${X86_64_SIMD}" STREQUAL "avx512")
    file(GLOB ASSEMBLY_SRC ${NNACL_DIR}/intrinsics/sse/*.c
            ${NNACL_DIR}/intrinsics/avx/*.c
            ${NNACL_DIR}/assembly/avx/*.S
            ${NNACL_DIR}/intrinsics/ms_simd_cpu_info.c)
    set_property(SOURCE ${ASSEMBLY_SRC} PROPERTY LANGUAGE C)

    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        file(GLOB HPC_SRC ${NNACL_DIR}/experimental/HPC-generator/gemm_avx512/*.c)
        set_property(SOURCE ${HPC_SRC} PROPERTY LANGUAGE C)
    endif()
endif()

if(APPLE)
    set_source_files_properties(${ASSEMBLY_SRC} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
endif()

########################### build nnacl library ########################
string(REPLACE "-fvisibility=hidden" "-fvisibility=default" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

add_library(nnacl_mid OBJECT ${KERNEL_SRC} ${TRAIN_SRC} ${ASSEMBLY_SRC} ${HPC_SRC})

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions(nnacl_mid PRIVATE ENABLE_DEBUG)
endif()

if(ENABLE_CPU)
    if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "aarch64")
        target_compile_definitions(nnacl_mid PRIVATE ENABLE_ARM ENABLE_ARM64 ENABLE_NEON)
        target_compile_options(nnacl_mid PRIVATE -ffast-math -flax-vector-conversions)
    elseif("${X86_64_SIMD}" STREQUAL "sse")
        target_compile_definitions(nnacl_mid PRIVATE ENABLE_SSE)
    elseif("${X86_64_SIMD}" STREQUAL "avx")
        target_compile_definitions(nnacl_mid PRIVATE ENABLE_SSE ENABLE_AVX)
    elseif("${X86_64_SIMD}" STREQUAL "avx512")
        target_compile_definitions(nnacl_mid PRIVATE ENABLE_SSE ENABLE_AVX ENABLE_AVX512)
        target_compile_options(nnacl_mid PRIVATE -mavx512f)
    endif()
    target_compile_options(nnacl_mid PRIVATE -fPIC)
    add_library(nnacl SHARED $<TARGET_OBJECTS:nnacl_mid>)
    if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
        if(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
            target_link_options(nnacl PRIVATE -Wl,-z,relro,-z,now,-z,noexecstack)
        endif()
        if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
            target_link_options(nnacl PRIVATE -s)
        endif()
    endif()
endif()

########################### arm fp16 build optimize library ########################
if(PLATFORM_ARM)
    add_subdirectory(${NNACL_DIR}/optimize)
endif()
