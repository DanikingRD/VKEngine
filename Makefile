include config.mk

SRC:= $(shell find ${SRC_DIR} -type f -name "*.c")
TEST_SRC:= $(shell find ${TEST_SRC_DIR} ${TEST_LIB_DIR} -type f -name "*.c")

OBJ_FROM_SRC:= $(patsubst ${SRC_DIR}/%.c,${OBJ_DIR}/%.o, ${SRC})
OBJ_FROM_TEST_SRC:= $(filter-out ${OBJ_DIR}/main.o, ${OBJ_FROM_SRC})
OUTPUT:= ${BUILD_DIR}/${PROJECT_NAME}

all: ${BUILD_DIR} ${OBJ_FROM_SRC}
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJ_FROM_SRC} -o ${OUTPUT} 

run: all
	@${OUTPUT}

test: ${BUILD_DIR} ${OBJ_FROM_TEST_SRC}
	${CC} ${CFLAGS_TEST} ${OBJ_FROM_TEST_SRC} ${TEST_SRC} -o ${OUTPUT}-test
	@${OUTPUT}-test

fmt: 
	@clang-format -i ${SRC} ${TEST_SRC}
	
${OBJ_DIR}/%.o: ${SRC_DIR}/%.c
	@mkdir -p $(dir $@)
	${CC} ${CFLAGS} -c -o $@ $<

${BUILD_DIR}:
	@mkdir ${OBJ_DIR}
	@mkdir ${BUILD_DIR}

clean: 
	@rm -rfv ${OBJ_DIR}
	@rm -rfv ${BUILD_DIR}
	@rm -rfv ${OBJ_FROM_SRC}

.PHONY: clean
