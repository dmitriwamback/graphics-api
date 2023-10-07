package main
import (
	"C"
	"unsafe"
)

func LoadModel() *C.float {

	vertex := make([]C.float, 100);
	vertexMemory := C.malloc(C.size_t(len(vertex)) * C.size_t(unsafe.Sizeof(uintptr(0))));
}