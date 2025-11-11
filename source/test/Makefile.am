##########################################################################
# If not stated otherwise in this file or this component's LICENSE
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
AM_CFLAGS =

AM_CXXFLAGS = -std=c++11

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = notify_comp_gtest.bin

notify_comp_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest \
                                 -I${top_srcdir}/gtest/include \
                                 -I${top_srcdir}/source \
                                 -I${top_srcdir}/source/include \
                                 -I${top_srcdir}/source/NotifyComponent \
                                 -DUNIT_TEST_DOCKER_SUPPORT

notify_comp_gtest_bin_SOURCES = gtest_main.cpp \
                                ${top_srcdir}/source/NotifyComponent/cosa_apis_NotifyComponent.c \
                                notify_comp_mock.cpp \
                                cosa_apis_NotifyComponent_test.cpp

notify_comp_gtest_bin_LDFLAGS = -lgtest -lgmock -lgcov -lpthread
notify_comp_gtest_bin_LDADD = $(HOME)/usr/local/lib/libmock_ccsp_common.la \
                              $(HOME)/usr/local/lib/libmock_rbus.la \
                              $(HOME)/usr/local/lib/libmock_safec_lib.la \
                              $(HOME)/usr/local/lib/libmock_pthread.la \
                              $(HOME)/usr/local/lib/libmock_file_io.la \
                              $(HOME)/usr/local/lib/libmock_mqhandler.la \
                              $(HOME)/usr/local/lib/libmock_ccsp_common.la

#Adding code coverage flags
AM_CFLAGS += -fprofile-arcs -ftest-coverage
AM_CXXFLAGS += -fprofile-arcs -ftest-coverage
notify_comp_gtest_bin_LDFLAGS += --coverage

