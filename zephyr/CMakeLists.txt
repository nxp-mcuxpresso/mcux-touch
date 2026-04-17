# Copyright 2025 NXP
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_NXP_TOUCH)
        zephyr_library_named(nxp_touch)
    
else() # CONFIG_NXP_TOUCH=n
    message("NXP Touch library excluded from the build. ")
endif()