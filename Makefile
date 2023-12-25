include config.mk

SRC:= $(shell find ${SRC_DIR} -type f -name "*.c")
TEST_SRC:= $(shell find ${TEST_SRC_DIR} ${TEST_LIB_DIR} -type f -name "*.c")

OBJ_FROM_SRC:= $(patsubst ${SRC_DIR}/%.c,${OBJ_DIR}/%.o, ${SRC})
OBJ_FROM_TEST_SRC:= $(filter-out ${OBJ_DIR}/main.o, ${OBJ_FROM_SRC})
OUTPUT:= ${BUILD_DIR}/${PROJECT_NAME}

all: ${BUILD_DIR} ${OBJ_FROM_SRC} shaders
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJ_FROM_SRC} -o ${OUTPUT} 

run: all
	@${OUTPUT}

test: ${BUILD_DIR} ${OBJ_FROM_TEST_SRC}
	${CC} ${CFLAGS_TEST} ${OBJ_FROM_TEST_SRC} ${TEST_SRC} -o ${OUTPUT}-test
	@${OUTPUT}-test

fmt: 
	@clang-format -i ${SRC} ${TEST_SRC}

shaders: 
	@rm -rfv bin/assets/shaders
	@echo "Compiling shaders..."
	@mkdir -p bin/assets/shaders
	@glslc -fshader-stage=vert assets/shaders/builtin.shader.vert.glsl -o bin/assets/shaders/builtin.shader.vert.spv
	@glslc -fshader-stage=frag assets/shaders/builtin.shader.frag.glsl -o bin/assets/shaders/builtin.shader.frag.spv
	@echo "Done."
		
${OBJ_DIR}/%.o: ${SRC_DIR}/%.c
	@mkdir -p $(dir $@)
	${CC} ${CFLAGS} -c -o $@ $<

${BUILD_DIR}:
	@mkdir -p ${OBJ_DIR}
	@mkdir -p ${BUILD_DIR}

clean: 
	@rm -rfv bin/assets/shaders
	@rm -rfv ${OBJ_DIR}
	@rm -rfv ${BUILD_DIR}
	@rm -rfv ${OBJ_FROM_SRC}

.PHONY: clean
