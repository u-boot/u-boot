#
#      Author: Xilinx, Inc.
#      
#      
#      This program is free software; you can redistribute it and/or modify it
#      under the terms of the GNU General Public License as published by the
#      Free Software Foundation; either version 2 of the License, or (at your
#      option) any later version.
#      
#      
#      XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
#      COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
#      ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
#      XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
#      FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
#      ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
#      XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
#      THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
#      WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
#      CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
#      FITNESS FOR A PARTICULAR PURPOSE.
#      
#      
#      Xilinx hardware products are not intended for use in life support
#      appliances, devices, or systems. Use in such applications is
#      expressly prohibited.
#      
#      
#      (c) Copyright 2002-2004 Xilinx Inc.
#      All rights reserved.
#      
#      
#      You should have received a copy of the GNU General Public License along
#      with this program; if not, write to the Free Software Foundation, Inc.,
#      675 Mass Ave, Cambridge, MA 02139, USA.
#
# Globals
lappend drvlist
set ltypes "../../../sw_services/uboot_v1_00_a/data/Ltypes"

proc uboot_drc {lib_handle} {
    puts "U-Boot DRC..."
}

proc generate {libname} {
    
    global drvlist
    
    # Get list of peripherals connected to uboot
    set conn_periphs [xget_handle $libname "ARRAY" "connected_periphs"]
    #lappend drvlist
    if {[string compare -nocase $conn_periphs ""] != 0} {
	set conn_periphs_elems [xget_handle $conn_periphs "ELEMENTS" "*"]
	# For each periph
	foreach periph_elem $conn_periphs_elems {
	    set periph [xget_value $periph_elem "PARAMETER" "periph_name"]
	    # 1. Get driver
	    set drv [xget_swhandle $periph]
	    set posn [lsearch -exact $drvlist $drv]
	    if {$posn == -1} {
		lappend drvlist $drv
	    }
	}
	
	set file_handle [xopen_include_file "xparameters.h"]
	puts $file_handle "\n/******************************************************************/\n"
	puts $file_handle "/* U-Boot Redefines */"
	puts $file_handle "\n/******************************************************************/\n"
	close $file_handle
	
	foreach drv $drvlist {
	    set drvname [xget_value $drv "NAME"]
	    
	    #Redefines xparameters.h
	    if {[string compare -nocase $drvname "uartns550"] == 0} {
		xredefine_uartns550 $drv "xparameters.h"
	    }  elseif {[string compare -nocase $drvname "emac"] == 0} {
		xredefine_emac $drv "xparameters.h"
	    }  elseif {[string compare -nocase $drvname "iic"] == 0} {
		xredefine_iic $drv "xparameters.h"
	    }
	}
    }
    
    # define core_clock
    xredefine_params $libname "xparameters.h" "CORE_CLOCK_FREQ_HZ"

    # define the values for the persistent storage in IIC
    xredefine_params $libname "xparameters.h" "IIC_PERSISTENT_BASEADDR" "IIC_PERSISTENT_HIGHADDR" "IIC_PERSISTENT_EEPROMADDR"

}

proc xget_corefreq {} {
    set processor [xget_processor] 
    set name [xget_value $processor "NAME"]
    puts "procname : $name"
    set processor_driver [xget_swhandle [xget_value $processor "NAME"]]
    puts "procdrv : $processor_driver"
    if {[string compare -nocase $processor_driver ""] != 0} {
	set arg "CORE_CLOCK_FREQ_HZ"
	#set retval [xget_value $processor_driver "PARAMETER" $arg]
	set retval [xget_dname [xget_value $processor_driver "NAME"] $arg]
	return $retval
    }
}

# procedure that adds # defines to xparameters.h as XPAR_argument
proc xredefine_params {handle file_name args} {
    
    puts "xredfine ..."
    # Open include file
    set file_handle [xopen_include_file $file_name]
    puts "args : $args"

    foreach arg $args {
	if {[string compare -nocase $arg "CORE_CLOCK_FREQ_HZ"] == 0} {
	    set value [xget_corefreq]
	    puts "corefreq : $value"
	} else {
	    set value [xget_value $handle "PARAMETER" $arg]
	    puts "value : $value"
	}
	
	if {$value != ""} {
	    set value [xformat_addr_string $value $arg]

	    if {[string compare -nocase $arg "IIC_PERSISTENT_BASEADDR"] == 0} {
		set name "PERSISTENT_0_IIC_0_BASEADDR"
	    } elseif {[string compare -nocase $arg "IIC_PERSISTENT_HIGHADDR"] == 0} {
		set name "PERSISTENT_0_IIC_0_HIGHADDR"
	    } elseif {[string compare -nocase $arg "IIC_PERSISTENT_EEPROMADDR"] == 0} {
		set name "PERSISTENT_0_IIC_0_EEPROMADDR"
	    } else {
		set name [string toupper $arg]
	    }
	    set name [format "XPAR_%s" $name]
	    puts $file_handle "#define $name $value"
	}
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

# uart redefines...
proc xredefine_uartns550 {drvhandle file_name} {
    
    xredefine_include_file $drvhandle $file_name "uartns550" "C_BASEADDR" "C_HIGHADDR" "CLOCK_HZ" "DEVICE_ID"
    
}

proc xredefine_emac {drvhandle file_name} {
    
    xredefine_include_file $drvhandle $file_name "emac" "C_BASEADDR" "C_HIGHADDR" "C_DMA_PRESENT" "C_MII_EXIST" "C_ERR_COUNT_EXIST" "DEVICE_ID"
    
}

proc xredefine_iic {drvhandle file_name} {
    xredefine_include_file $drvhandle $file_name "iic" "C_BASEADDR" "C_HIGHADDR" "C_TEN_BIT_ADR" "DEVICE_ID"

}

#######################

proc xredefine_include_file {drv_handle file_name drv_string args} {
    
    # Open include file
    set file_handle [xopen_include_file $file_name]
    
    # Get all peripherals connected to this driver
    set periphs [xget_periphs $drv_handle] 
    
    set pname [format "XPAR_%s_" [string toupper $drv_string]]
    
    # Print all parameters for all peripherals
    set device_id 0
    set sub_periphs 1
    foreach periph $periphs {
	puts "$periph : $drv_string : $sub_periphs"

	for {set i 0} {$i < $sub_periphs} {incr i} {
	    foreach arg $args {
		set name "${pname}${device_id}_"
	    
		if {[string compare -nocase "CLOCK_HZ" $arg] == 0} {
		    set xdrv_string [format "%s%s" "X" $drv_string]
		    set value [xget_dname $xdrv_string $arg]
		    set name "${name}CLOCK_FREQ_HZ"
		} else {
		    if {[string match C_* $arg]} {
			set name [format "%s%s" $name [string range $arg 2 end]]
		    } else {
			set name "${name}${arg}"
		    }
		    set value [xget_name $periph $arg]
		}

		if {[string compare -nocase "uartns550" $drv_string] == 0} {
		    if {[string compare -nocase "C_BASEADDR" $arg] == 0} {
			set value [format "(%s%s%s)" $value "+" "0x1000"]
		    }
		}

		puts $file_handle "#define $name $value"
		if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
		    incr device_id
		}
	    }
	}
    }		
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

##################################################
# procedure post_generate
# This generates the drivers directory for uboot
# and runs the ltypes script
##################################################

proc post_generate {lib_handle} {
    
    global drvlist
    
    # Create U-Boot tree structure
    set pwd [pwd]
    set common_dir "uboot/board/xilinx/common"
    set xilinx_enet_dir "uboot/board/xilinx/xilinx_enet"
    set ml300_dir  "uboot/board/xilinx/ml300"
    
    exec bash -c "mkdir -p $common_dir $xilinx_enet_dir $ml300_dir"
    
    # Copy files for xilinx_ocp
    xcopy_commonfiles

    foreach drv $drvlist {
	set drvname [xget_value $drv "NAME"]
	set ver [xget_value $drv "PARAMETER" "DRIVER_VER"]
	set ver [string map {. _} $ver]
	set dirname [format "%s_v%s" $drvname $ver]
	
	if {[string compare -nocase $drvname "emac"] == 0} {
	    xcopy_emac $drv $dirname
	} elseif {[string compare -nocase $drvname "iic"] == 0} {
	    xcopy_iic $drv $dirname
	}
    }
    
    # Call Ltypes Script here
    set uboot "uboot"
    xltype_file $uboot

    # Move xparameters.h around
    exec bash -c "cp ../../include/xparameters.h $ml300_dir"

    # copy the whole U-Boot BSP to its final destination
    set value [xget_value $lib_handle "PARAMETER" TARGET_DIR]
    puts "TARGET_DIR : $value"

    if {$value != ""} {
        if {[file isdirectory $value] == 0} {
            exec bash -c "mkdir -p $value"
        }
        exec bash -c "cp -Rp uboot/* $value"
    }
}

proc xcopy_commonfiles {} {

    global drvlist

    set common_dir "uboot/board/xilinx/common"
    
    foreach drv $drvlist {
	set depends [xget_value $drv "OPTION" "DEPENDS"]
	foreach dep $depends {
	    puts "dep : $dep"
	    if {[file isdirectory "../$dep"] == 1} {
		exec bash -c "cp -f ../$dep/src/*.c $common_dir"
		exec bash -c "cp -f ../$dep/src/*.h $common_dir"
	    }
	}
    }
    
}

proc xcopy_emac {drv_handle dirname} {
    set emac "board/xilinx/xilinx_enet"
    xcopy_dir $dirname $emac
}

proc xcopy_iic {drv_handle dirname} {
    set iic "board/xilinx/xilinx_iic"
    xcopy_dir $dirname $iic
}

proc xcopy_dir {srcdir dstdir} {
    
    set dstdirname [format "%s%s" "uboot/" $dstdir]
    if {[file isdirectory "../$srcdir"] == 1} {
	# Copy files from src to dst
	exec bash -c "mkdir -p $dstdirname"
	exec bash -c "cp -f ../$srcdir/src/*.c $dstdirname"
	exec bash -c "cp -f ../$srcdir/src/*.h $dstdirname"
    } else {
	puts "$srcdir does not exist ..."
    }
}


proc xltype_file {filename} {

    global ltypes

    puts $filename

    if {[file isdirectory $filename]} {
	foreach entry [glob -nocomplain [file join $filename *]] {
	    xltype_file $entry
	}
    } else {
	exec bash -c "$ltypes $filename"
    }
    
}
