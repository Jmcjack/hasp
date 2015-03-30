/*! \file doxygen.h
 *	\brief Convenient spot for doxygen text and group definitions.
 *
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: doxygen.h 770 2012-02-14 15:32:39Z murch $
 */

// Pages
/*! \mainpage Table of Contents
 *	The following pages give a description of the UMN UAV Software.
 *	-# \subpage overview
 *	-# \subpage sched
 *	-# \subpage control
 *	-# \subpage datalog
 *	-# \subpage compile_guide
 *
 *	<a href="http://www.uav.aem.umn.edu/">Back to UAV home page</a>
 */

 /*! \page overview Overview
 * The onboard flight computer utilizes a real-time operating system (eCos) and flight software written in C. In addition
 * to being open source and freely available, the eCos operating system provides a real-time kernel, can be
 * configured to minimize the computing overhead required for the operating system, supports multi-threading, and is POSIX
 * C compatible.
 *
 * The flight software is composed of ten distinct <a href="modules.html">modules</a>, shown in the following block diagram (\ref flightcode_bd "Figure 1").
 * Each module has a standardized interface, allowing different implementations (e.g., different control laws) to be easily interchanged
 * at compile time. Optional modules are colored gray, and are not required for normal flight operations. In addition to the
 * module interfaces, each sensor driver uses a standardized interface, which enables the same code base to be configured
 * for aircraft with different sensor configurations without modifying the source code. Modules outlined in red are included in the
 * software-in-the-loop simulation.
 *
 *  \image html FlightCode_block_diagram_small.png "Figure 1: Block diagram of flight software"
 *  \anchor flightcode_bd
 *
 *  The majority of variables are contained in data structures (see <a href="annotated.html">Data Structures List</a> for details).
 *  Pointers to these data structures are passed to each module to provide access to variables. The data structures are global in
 *  scope to enable sharing between threads. Mutexes or some form of handling will need to be implemented to prevent resource
 *  conflicts.
 */

 /*! \page sched Scheduling & Threads
 * The flight software uses a multi-threaded architecture in which all of the flight critical tasks execute in the highest
 * priority thread at 50Hz (Thread 0 in \ref flightcode_bd "software block diagram"), while additional tasks (i.e., those not required to
 * control the aircraft, such as a fault detection filter) are executed in separate, lower priority threads. Alarm functions
 * provided by the RTOS kernel are used to trigger the threads, and are also used to hold the flight critical thread at key
 * execution points to ensure each module of the software executes at a deterministic sample rate. The flight critical
 * modules are executed in order shown in the \ref flightcode_bd "software block diagram", beginning with the data acquisition (DAQ). See the module
 * <a href="group__sched__fcns.html">Scheduling</a> for details. The timing of the alarms is set in scheduling.h.
 *
 *  \image html FlightCode_timing_diagram_small.png "Figure 1: Timing diagram of flight software"
 *  \anchor flightcode_timing
 *
 * \section Threads
 * Additional threads can be added to the flight software easily using pthread.h functions. Execution of the threads is controlled
 * by RTOS kernel alarms which set trigger condition variables.
 */

/*! \page control Control Laws
 * The control laws for the UMN UAV aircraft can be written in either C code or Simulink, but are ultimately implemented in C
 * code. The same control code is used by the Software in the Loop simulation (SIL), Processor in the Loop simulation (PIL),
 * and flight code. Control codes use a standard function interface described in detail <a href="group__control__fcns.html">here</a>.
 *
 * \section excodes Example Control Codes
 * Examples and baseline code can be found in the folder trunk/Software/FlightCode/control/; a brief explanation about these
 * implementations is given as follows:
 *
 * empty_control.c:  Provides a basic layout for the control code.
 *
 * baseline_control.c: Baseline PI controller which uses angles and angular rate feedback. It has an integral action on the
 * position errors (angles) and an integrator anti-windup strategy to handle actuator saturation. Has a simple yaw damper for the directional axis.
 *
 * lqr_control.c: LQR controller that uses angles and angular rate feedback and an integrator anti-windup strategy to handle
 * actuator saturation. Lateral-direction axes (aileron-rudder) are coupled.
 *
 * \section rtw Simulink Autocode Generation
 * Control codes can be written in Simulink and then transformed into C code using Simulink Coder (formerly Real Time Workshop).
 * This section detail the necessary steps for successfully integrating a Simulink control law into the flight code. The simplest
 * way to do this is to copy and modify the baseline_control.mdl diagram, which is already properly configured for autocoding.
 *
 * \subsection rtw_interface 1. Interface
 * The first step is to ensure the Simulink diagram uses the proper input and output interface. Refer to the
 * <a href="http://svn.umnaem.webfactional.com/trunk/Software/Documentation/UAV_controllaw_ICD.pdf">Control Law ICD</a> for details.
 * There are two root-level inports: \e feedback and \e ref_cmds, which provide sensor data and control law reference commands,
 * respectively. There are two root-level outports: \e control_cmd and \e ref_cmds_out. The latter is not used by the flight code,
 * but is necessary for SIL simulation.
 *
 * \subsection rtw_config 2. Model Configuration Parameters
 * The second step is to set the Simulink Configuration Parameters. The following screenshots show the proper configuration.
 *
 * The first setting to change is under \e Hardware \e Implementation. The Device vendor should be set to 'Freescale' and the
 * Device type should be set to '32-bit PowerPC'.
 *
 * \image html rtw_config_hw.png "Figure 1: Screenshot of Hardware Implementation page settings"
 * \anchor rtw_config_hw
 *
 * The second setting to change is under \e Real-Time \e Workshop. Make sure the System target file is 'grt.tlc' and the 'Generate
 * code only' box is checked.
 *
 * \image html rtw_config_rtw.png "Figure 2: Screenshot of Real-Time Workshop page settings"
 * \anchor rtw_config_rtw
 *
 * The last setting to change is under \e Custom \e Code. The file /FlightCode/control/rtw_grt_control.c should be included in the
 * Source file using a preprocessor include statement.
 *
 * \image html rtw_config_custom.png "Figure 3: Screenshot of Custom Code page settings"
 * \anchor rtw_config_custom
 *
 * At this point the model can be 'built' using 'Ctrl+b'. This will place the C code in a folder called MODELNAME_grt_rtw in your
 * MATLAB working directory.
 *
 * \subsection rtw_intg 3. Compiling the Flight Code
 * The final step is to compile the flight code with the autogenerated control code. This will require editing the autocode and the
 * \e Makefile.
 *
 * Open the header file for the autocode (MODELNAME.h). Comment out  the following lines:
 *
 *	<tt>
 * 	#include "simstruc.h"   \n
 * 	#include "fixedpoint.h" </tt>
 *
 * This is necessary as these headers have conflicting define statements with math.h. These headers are not needed for the flight
 * code, but are included as part of the RTW Generic Real-time Target (GRT) package.
 *
 * Open the \e Makefile and find the section titled MATLAB RTW AUTOCODE.  Specify the path to the autocode files, as
 * well as the names of all of the C source files in that directory that will need to be compiled. Note that in addition to the
 * primary autocode file (MODELNAME.c) there will be additional utility files that must be compiled. See below for an example:
 *
 * <tt>##### MATLAB RTW AUTOCODE #####	\n
 * # Make sure #include "simstruc.h" and #include "fixedpoint.h" are commented out in the <modelname>.h file \n
 * # Specify the path to the autogenerated code, e.g. /<modelname>_grt_rtw/ \n
 * RTW_ROOT = ../../Simulation/SIL_sim/baseline_control_grt_rtw/ \n
 *
 * # Specify the main code to be compiled <modelname>.c, along with any utility source files \n
 * CONTROL = $(RTW_ROOT)baseline_control.c \ \n
 * $(RTW_ROOT)rtGetInf.c \ \n
 * $(RTW_ROOT)rtGetNaN.c \ \n
 * $(RTW_ROOT)rt_nonfinite.c \n </tt>
 *
 * Finally, make sure the path to the MATLAB installation is correct in the section of the \e Makefile titled DIRECTORY SETUP.
 *
 * <tt>##### MATLAB RTW ##### \n
 * # Path to your MATLAB directory for RTW files. \n
 * # Cannot have any spaces in the names! \n
 * # Use /cygdrive/c/ instead of C:/ \n
 * MATLAB_ROOT = /cygdrive/c/PROGRA~2/MATLAB/R2010a \n
 *
 * MATLAB_INCLUDES = \ \n
 * 	-I$(MATLAB_ROOT)/simulink/include \ \n
 * 	-I$(MATLAB_ROOT)/extern/include \ \n
 * 	-I$(MATLAB_ROOT)/rtw/c/src  \n </tt>
 *
 */

/*! \page datalog Data Logging
 *	Data is stored onboard in RAM memory in MATLAB .mat file format (v4). The data file is transmitted to the host PC from the
 *	flight computer over Ethernet via TFTP. The file name will be "flightdata_tXXX.XXXX.mat" where the X is replaced by the
 *	system time in seconds. There is no file overwrite protection.
 *
 *	Configuring what data are stored is done via header files in the FlightCode/datalog/ directory (e.g. standard_datalog.h)
 *	for variables that exist in main(). Other variables, such as those in an auxiliary thread, can be added to the datalogging
 *	via functions described below. This step must be done prior to any waiting or timing functions.
 *	The header files define four arrays of pointers to the variables that will be stored:
 *		- saveAsDoublePointers: double pointer array to variables that will be saved as doubles
 *		- saveAsFloatPointers: \e double pointer array to variables that will be saved as floats (not a typo)
 *		- saveAsIntPointers: int32_t pointer array to variables that will be saved as ints
 *		- saveAsShortPointers: uint16_t pointer array to variables that will be saved as shorts
 *
 *	There are four additional char arrays that are the corresponding variable names for each pointer. Note that these names
 *	must be valid MATLAB variable names.
 *		- saveAsDoubleNames: char array of variable names for doubles
 *		- saveAsFloatNames: char array of variable names for floats
 *		- saveAsIntNames: char array of variable names for ints
 *		- saveAsShortNames: char array of variable names for shorts
 *
 *	Finally, there are five define statements that set the length of the data logging array and the number
 *	of variables in each pointer array.
 *
 *	Variables outside of the scope of main() can be added to each respective arrary with the following functions:
 *		- add_double_datalog()
 *		- add_float_datalog()
 *		- add_int_datalog()
 *		- add_short_datalog()
 *
 *	Important considerations:
 *		- Variables must be global in scope to be logged
 *		- Save double variables as float unless absolutely necessary (memory limitation). Most variables in the FlightCode
 *		are double, but can be saved as float.
 *		- See the <a href="group__datalog__fcns.html">Datalogging Module</a> for more details
 *
 */

/*! \page compile_guide Compile Guide
 * This page describes how to compile, load, and run software on the UAV flight computer. The following figure illustrates
 * the overall system setup.
 *
 *  \image html compileguide_setup.png "Figure 1: Flight Computer and Host PC Setup"
 *  \anchor overall_setup
 *
 *  Users interface with the flight computer operating system (Redboot) in one of two ways: 1) RS-232 serial, or 2) Ethernet.
 *  The flight software also uses the telemetry system (which is TTL serial) to communicate with the Host PC. Flight software
 *  is compiled in Cygwin using the \e make tool. The eCos operating system (v3.0) and the MPC5200B libraries must be installed
 *  to compile the flight code. Compiled binaries are uploaded onto the flight computer over Ethernet using the TFTP protocol
 *  (Tftpd-32 is used as a TFTP server on the host PC). Data logged onboard the flight computer is downloaded to the host PC
 *  in a similar fashion.
 *
 *  \section req_prog Required Programs
 *	The following programs are required for compiling software for the MPC5200B. See the appendix for details on obtaining and installing these programs.
 *	- \b Cygwin: A GNU/Linux emulator for windows that is used to compile the flight software.
 *	- \b eCos (Embedded configurable operating system) version 3.0:  The eCos development tools for power-pc must be installed. In addition, the libraries and utilities for the MPC5200B must be available.
 *	- \b TFTP server: Tftpd-32 software is used to transfer data between the PC and the flight computer via Ethernet by using the TFTP protocol.
 *	- \b Terminal: Hyperterminal, PuTTY, or any other terminal program can be used to communicate with the embedded system over serial or Ethernet. The OpenUGS ground station enables serial communication.
 *	- \b TortoiseSVN (recommended): Tool for interfacing with the SVN repository where the flight code and eCos libraries are stored.
 *	- \b Eclipse IDE (recommended): An integrated software development tool that integrates the cygwin compiler, SVN, and doxygen.
 *
 *	\section download_code Download the flight code
 *	Download the flight code using TortoiseSVN into a directory of your choice. Right-click and select <em> SVN Checkout... </em>. The following screen should appear:
 *
 *	\image html compileguide_svncheckout.png "Figure 2: Screenshot of SVN Checkout"
 *  \anchor svncheckout
 *
 *  The URL of the main repository is: http://svn.umnaem.webfactional.com/ but only /trunk/Software/ is required to compile the flight code.
 *
 *  \section compile Compile the Flight Code
 *	Building the flight code uses a \e makefile, which is how the flight code is configured for different airplanes and operational modes. The makefile is essentially a recipe for the compiler, telling it what to compile and where to find it.
 *	-# The compiler needs to know where to find the eCos libraries. Edit the section of the makefile labeled MPC 5200 PARAMETERS, around line 150. Make sure the paths to the eCos PowerPC folders are correct:
 *		- POWER_PC_CC=/opt/ecos/gnutools/powerpc-eabi/bin/powerpc-eabi-gcc
 *		- CP=/opt/ecos/gnutools/powerpc-eabi/bin/powerpc-eabi-objcopy
 *		- OD=/opt/ecos/gnutools/powerpc-eabi/bin/powerpc-eabi-objdump
 *		.
 *	Then, modify the paths to the eCos libraries for the MPC5200B:
 *		- ECOS_LIB_PATH_MPC5200=/trunk/Software/eCos/build/lib
 *		- ECOS_INCLUDE_PATH_MPC5200=/trunk/Software/eCos/build/include
 *		.
 *	Alternatively, the paths to the eCos libraries can be set in your .bash_profile, so they will be globally available to Cygwin.
 *	-# Edit the makefile to select the aircraft, guidance, navigation, control, system ID, and fault codes that will be compiled.
 *	-# Next, start Cygwin and change directories to where you downloaded the flight code from the SVN repository.
 *	-# Compile the flight code using the command make. Below is an example output:
 *
 *	\image html compileguide_cygwin.png "Figure 3: Screenshot of Successful Compile Using Cygwin"
 *  \anchor cgywin
 *
 *  Note the output that describes what source files were compiled. If no compilation errors are displayed, the binary file called output will be generated. This file is the executable which will be loaded onto the flight computer. To remove all the files generated from the compilation type: make clean.
 *
 *  \section upload Upload the binary file to the UAV flight computer
 *	- Connect an Ethernet cable from the MPC5200B to the Host PC.
 *	- Verify the Host PC IP settings as shown in the following figure:
 *
 *  \image html compileguide_ipsettings.png "Figure 4: Screenshot of IP Settings"
 *  \anchor ipsettings
 *
 *  - Start the TFTP server (Tftpd-32) and select the folder where the compiled binary file are be located. Usually this is the same source code directory (Software/FlightCode).
 *
 *  \image html compileguide_tftp.png "Figure 5: Screenshot of TFTP server"
 *  \anchor tftp
 *
 *  - Open the terminal program (Hyperterminal, PuTTY or ground station) and configure the connection
 *		- For serial connection to the UAV flight computer (COM0 RS232), use the following settings:
 *			Port name: COM2 (Host PC) \n
 *			Bit rate: 115200	\n
 *			Data bits: 8	\n
 *			Stop bits: 1	\n
 *			Parity: none	\n
 *			Flow control: none	\n
 *		.
 *		- For Ethernet connection only, use PuTTY or telnet, with the following settings:
 *			Host name: 192.168.3.11 \n
 *			Port: 9000	\n
 *			Note: the flight computer must be powered on before you can connect via Ethernet.
 *
 *	- Power on the flight computer and avionics. The terminal should display the RedBoot> prompt.
 *	- Upload the binary file from the host computer using \e load \e output
 *	- The terminal should display something similar to: \n
 *		***Loadable segment: 0, Len=479368	\n
 *		Entry point: 0x00100100, address range: 0x00100000-0x00175088	\n
 *		RedBoot>	\n
 *	    The binary file is now loaded in RAM memory.
 *	- Run the program using the command \e go.
 *
 *	\section download Downloading data from the UAV flight computer
 *	- Make sure the Ethernet cable is connected to the flight computer.
 *	- Press and hold the \e Dump button above the Ethernet port on the UAV for 3 seconds.
 *	- A MATLAB file will be downloaded to the current directory of the Tftpd-32 server. There is no overwrite protection, so make sure you rename each file after it is downloaded!
 *
 *	\section appendix Appendix
 *	- Cygwin: http://cygwin.com/index.html  See the instructions on the eCos website: http://ecos.sourceware.org/cygwin.html
 *	- eCos 3.0:  http://ecos.sourceware.org/getstart.html Be sure to get the PowerPC toolchain.
 *	- TFTP server: http://tftpd32.jounin.net/tftpd32_download.html Select the “tftpd32 standard edition”
 *	- Terminal: PuTTY: http://the.earth.li/~sgtatham/putty/latest/x86/putty.exe
 *	- TortoiseSVN: http://tortoisesvn.net/downloads.html
 *	- Eclipse IDE: http://www.eclipse.org/
 */

// Group Definitions
/*!\defgroup nav_fcns Navigation
 * \brief This section includes all of the navigation functions and files. All navigation codes must
 *  include nav_interface.h and instantiate the init_nav(), get_nav(), and close_nav() functions.
 */

/*!\defgroup guidance_fcns Guidance
 * \brief This section includes all of the guidance law functions and files. All guidance laws must
 *  include guidance_interface.h and instantiate the get_guidance() function.
 */

/*!\defgroup control_fcns Control
 * \brief This section includes all of the control law functions and files. All control laws must
 *  include control_interface.h and instantiate the get_control() and reset_control() functions.
 */

/*!\defgroup fault_fcns Faults
 * \brief This section includes all of the surface and sensor fault functions and files. All faults must
 *  include fault_interface.h and instantiate the get_sensor_fault() or get_surface_fault() function.
 */

/*!\defgroup systemid_fcns System ID
 * \brief This section includes all of the system identification functions and files. All system ID codes must
 *  include systemid_interface.h and instantiate the get_system_id() function.
 */


/*!\defgroup daq_fcns Sensors
 * \brief This section includes all of the data acquisition and sensor functions and files. All sensor codes must
 *  include the appropriate interface header. All data acquisition codes must include daq_interface.h and instantiate
 *  the init_daq() and get_daq() functions.
 */

/*!\defgroup adc_fcns A/D Conv
 * \brief This section includes all of the analog to digital converter drivers and files. All ADC codes must include
 *  adc_interface.h and instantiate the init_adc() and read_adc() functions.
 *  \ingroup daq_fcns
 */

/*!\defgroup airdata_fcns Air Data
 * \brief This section includes all of the air data sensor drivers and files. All air data codes must include
 *  airdata_interface.h and instantiate the init_airdata() and read_airdata() functions.
 *  \ingroup daq_fcns
 */

/*!\defgroup gpio_fcns GPIO
 * \brief This section includes all of the GPIO drivers and files. All GPIO codes must include
 *  gpio_interface.h and instantiate the init_gpio() and read_gpio() functions.
 *  \ingroup daq_fcns
 */

/*!\defgroup gps_fcns GPS
 * \brief This section includes all of the GPS drivers and files. All GPS codes must include
 *  gps_interface.h and instantiate the init_gps() and read_gps() functions.
 *  \ingroup daq_fcns
 */

/*!\defgroup imu_fcns IMU
 * \brief This section includes all of the IMU drivers and files. All IMU codes must include
 *  imu_interface.h and instantiate the init_imu() and read_imu() functions.
 *  \ingroup daq_fcns
 */

/*!\defgroup pwm_fcns PWM
 * \brief This section includes all of the PWM drivers and files. All PWM codes must include
 *  pwm_interface.h and instantiate the init_pwm() and read_pwm() functions.
 *  \ingroup daq_fcns
 */

/*!\defgroup actuator_fcns Actuators
 * \brief This section includes all of the actuator functions and files. All actuator codes must
 *  include actuator_interface.h and instantiate the init_actuators(), set_actuators(), and close_actuators() functions.
 */

/*!\defgroup aircraft_cfg Aircraft Configuration
 * \brief This section includes all of the aircraft configuration header files, which set the geometry,
 *  surface calibrations and limits, and serial port configuration for the aircraft.
 */

/*!\defgroup datalog_fcns Data Logging
 * \brief This section includes all of the data logger functions and files. All datalogger codes must
 *  include datalog_interface.h and instantiate the init_datalogger(), datalogger(), and close_datalogger() functions.
 *  A header file that defines which variables will be stored must also be specified (e.g. standard_datalog.h).
 */

/*!\defgroup telemetry_fcns Telemetry
 * \brief This section includes all of the telemetry functions and files. All telemetry codes must
 *  include telemetry_interface.h and instantiate the init_telemetry() and send_telemetry() functions.
 */

/*!\defgroup sched_fcns Scheduling
 * \brief This section includes all of the scheduling and thread handling functions and files. Time offsets for alarms are set
 * in scheduling.h.
 */
