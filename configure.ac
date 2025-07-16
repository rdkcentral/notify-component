##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2019 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

dnl AC_PREREQ([2.69])
AC_INIT(NotifyComponent-rdkb, version-1.0)
AC_CONFIG_SRCDIR([])
AM_CONFIG_HEADER(cfg/config.h)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([cfg])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-bzip2 1.9 subdir-objects])
LT_INIT

AC_ARG_ENABLE([unitTestDockerSupport],
             AS_HELP_STRING([--enable-unitTestDockerSupport],[enable docker support for unit testing (default is no)]),
             [
              UNIT_TEST_DOCKER_SUPPORT=""
              case "${enableval}" in
               yes) UNIT_TEST_DOCKER_SUPPORT=true
                    m4_if(m4_sysval,[0],[AC_CONFIG_FILES([source/test/Makefile])]);;
               no) UNIT_TEST_DOCKER_SUPPORT=false AC_MSG_ERROR([Gtest support is disabled]);;
               *) AC_MSG_ERROR([bad value ${enableval} for --enable-unitTestDockerSupport ]);;
              esac
             ],
             [echo "Docker support is disabled"])
AM_CONDITIONAL([UNIT_TEST_DOCKER_SUPPORT], [test x$UNIT_TEST_DOCKER_SUPPORT = xtrue])
dnl subdirectories.
SUBDIRS=""

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CPP
AC_PROG_CXXCPP

dnl use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
  [AM_DEFAULT_VERBOSITY=1
   AC_SUBST(AM_DEFAULT_VERBOSITY)])




dnl **********************************
dnl checks for dependencies
dnl **********************************
AC_HEADER_STDC

dnl Checks for header files.
AC_CHECK_HEADERS([limits.h memory.h stdlib.h string.h sys/socket.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT8_T

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup strerror])

PKG_CHECK_MODULES([DBUS],[dbus-1 >= 1.6.18])

AC_CONFIG_FILES([Makefile
		source/Makefile
		source/NotifyComponent/Makefile])
AC_SUBST(UNIT_TEST_DOCKER_SUPPORT)
AC_OUTPUT
