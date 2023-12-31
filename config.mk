PROJECT_NAME:=explora
CC:=clang

SRC_DIR:=src
OBJ_DIR:=obj
BUILD_DIR:=bin
LIB_DIR:=lib
TESTS_DIR:=tests
TEST_LIB_DIR:=${TESTS_DIR}/lib
TEST_SRC_DIR:=${TESTS_DIR}/src

CBASE_FLAGS:=-Wall -Wextra -pedantic -g -std=c11 -D_DEBUG 
CWARNING_FLAGS:=-Wno-empty-translation-unit -Wno-missing-braces
# Compile flags
CFLAGS +=${CBASE_FLAGS}
CFLAGS +=-I${SRC_DIR} -I${VULKAN_SDK}/include ${CWARNING_FLAGS}
# Compile flags for testing
CFLAGS_TEST +=${CBASE_FLAGS} ${LDFLAGS}
CFLAGS_TEST +=-I${SRC_DIR} -I${TEST_SRC_DIR} -I${TEST_LIB_DIR} ${CWARNING_FLAGS}
# Link flags
LDFLAGS:= -lglfw -lm -lvulkan -L${VULKAN_SDK}/lib

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS += -rpath ${VULKAN_SDK}/lib
endif


