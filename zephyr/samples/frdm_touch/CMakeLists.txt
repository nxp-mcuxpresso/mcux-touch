# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(frdm_touch)

# Remove Freemaster
add_compile_definitions(NT_FREEMASTER_SUPPORT=0)

set(LIB_DIR ${ZEPHYR_BASE}/../modules/lib/nxp_touch)
set(SDK_DIR ${ZEPHYR_BASE}/../modules/hal/nxp/mcux/mcux-sdk-ng)
set(SDK_DRV_DIR ${ZEPHYR_BASE}/../modules/hal/nxp/mcux/mcux-sdk-ng/drivers)

target_include_directories(app PRIVATE src/)
target_include_directories(app PRIVATE src/boards)

# Include board specific files
if((CONFIG_BOARD STREQUAL frdm_ke15z))
    set(BOARD_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src/boards/frdm_ke15z)

elseif((CONFIG_BOARD STREQUAL frdm_ke16z))
    set(BOARD_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src/boards/frdm_ke16z)

elseif((CONFIG_BOARD STREQUAL frdm_ke17z))
    set(BOARD_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src/boards/frdm_ke17z)

elseif((CONFIG_BOARD STREQUAL frdm_ke17z512))
    set(BOARD_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src/boards/frdm_ke17z512)

else()
    message(FATAL_ERROR "Board is not supported by nxp_touch library module!")
endif()

target_sources(app PRIVATE
    src/main.c
    src/main.h
)

#Board specific config files
target_include_directories(app PRIVATE ${BOARD_DIR})

target_sources(app PRIVATE
    ${BOARD_DIR}/board.h
    ${BOARD_DIR}/nt_setup.h
    ${BOARD_DIR}/nt_setup.c
)

target_include_directories(app PRIVATE
    ${LIB_DIR}/source/controls
    ${LIB_DIR}/source/electrodes
    ${LIB_DIR}/source/filters
    ${LIB_DIR}/source/keydetectors
    ${LIB_DIR}/source/modules
    ${LIB_DIR}/source/system
    ${LIB_DIR}/source/safety
    ${LIB_DIR}/source/xtalk
    ${LIB_DIR}/source/drivers/tsi
    ${LIB_DIR}/include
    ${BOARD_DIR}/
)


# touch middleware files
target_sources(app PRIVATE
${LIB_DIR}/source/controls/nt_control_arotary.c
${LIB_DIR}/source/controls/nt_control_aslider.c
${LIB_DIR}/source/controls/nt_control_keypad.c
${LIB_DIR}/source/controls/nt_control_matrix.c
${LIB_DIR}/source/controls/nt_control_proxi.c
${LIB_DIR}/source/controls/nt_control_rotary.c
${LIB_DIR}/source/controls/nt_control_slider.c
${LIB_DIR}/source/controls/nt_controls.c
${LIB_DIR}/source/electrodes/nt_electrodes.c
${LIB_DIR}/source/filters/nt_filters.c
${LIB_DIR}/source/keydetectors/nt_keydetector_afid.c
${LIB_DIR}/source/keydetectors/nt_keydetector_safa.c
${LIB_DIR}/source/keydetectors/nt_keydetector_usafa.c
${LIB_DIR}/source/keydetectors/nt_keydetector_mbw.c
${LIB_DIR}/source/modules/nt_module_gpio.c
${LIB_DIR}/source/modules/nt_module_gpioint.c
${LIB_DIR}/source/modules/nt_module_tsi.c
${LIB_DIR}/source/modules/nt_modules.c
${LIB_DIR}/source/nt.c
${LIB_DIR}/source/system/nt_system.c
${LIB_DIR}/source/system/nt_system_frmstr.c
${LIB_DIR}/source/system/nt_system_mem.c
${LIB_DIR}/source/xtalk/nt_xtalk.c
${LIB_DIR}/source/safety/nt_safety_gpio.c
${LIB_DIR}/source/safety/nt_safety_crc.c
${LIB_DIR}/source/safety/nt_safety.c

${LIB_DIR}/source/drivers/tsi/nt_drv_tsi_common.c
${LIB_DIR}/source/drivers/tsi/nt_drv_tsi_driver.c
${LIB_DIR}/source/drivers/tsi/nt_drv_tsi_v5_driver_specific.c
${LIB_DIR}/source/drivers/tsi/nt_drv_tsi_v5_driver_specific.h

${SDK_DRV_DIR}/crc/fsl_crc.c
${SDK_DRV_DIR}/ftm/fsl_ftm.c
${SDK_DRV_DIR}/gpio/fsl_gpio.c
${SDK_DRV_DIR}/lpit/fsl_lpit.c
${SDK_DRV_DIR}/lpit/fsl_lpit.c
${SDK_DRV_DIR}/tsi/tsi_v5/fsl_tsi_v5.c

)

#------------------- ADD HAL SDK drivers -------------------------------------------------
include_directories(app PRIVATE ${SDK_DIR}/drivers/tsi/tsi_v5)
include_directories(app PRIVATE ${SDK_DIR}/drivers/ftm)
include_directories(app PRIVATE ${SDK_DIR}/drivers/crc)
include_directories(app PRIVATE ${SDK_DIR}/drivers/gpio)
#include_directories(app PRIVATE ${SDK_DIR}/devices/MKE15Z7)
#include_directories(app PRIVATE ${SDK_DIR}/devices/MKE17Z7)

#------------------- ADD application Source files -----------------------------------------
include_directories(app PRIVATE src)


target_sources(app PRIVATE 
                    src/main.c
#                   src/nt_setup.c
)
