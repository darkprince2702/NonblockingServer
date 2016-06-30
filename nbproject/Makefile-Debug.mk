#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=CLang-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Connection.o \
	${OBJECTDIR}/DemoServer.o \
	${OBJECTDIR}/IOHandler.o \
	${OBJECTDIR}/Processor.o \
	${OBJECTDIR}/Protocol.o \
	${OBJECTDIR}/Server.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs libevent` -lPocoFoundation  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nonblockingserver

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nonblockingserver: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nonblockingserver ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Connection.o: Connection.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libevent` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Connection.o Connection.cpp

${OBJECTDIR}/DemoServer.o: DemoServer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libevent` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DemoServer.o DemoServer.cpp

${OBJECTDIR}/IOHandler.o: IOHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libevent` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/IOHandler.o IOHandler.cpp

${OBJECTDIR}/Processor.o: Processor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libevent` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Processor.o Processor.cpp

${OBJECTDIR}/Protocol.o: Protocol.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libevent` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Protocol.o Protocol.cpp

${OBJECTDIR}/Server.o: Server.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `pkg-config --cflags libevent` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.o Server.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nonblockingserver

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
