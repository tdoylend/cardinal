#include <cardinal.h>

CAR_API Car_Version car_get_linked_version(void) {
	return CAR_COMPILED_VERSION;
}

CAR_API int car_compare_versions(Car_Version *a, Car_Version *b) {
	if (a->major > b->major) return  3;
	if (a->major < b->major) return -3;

	if (a->minor > b->minor) return  2;
	if (a->minor < b->minor) return -2;

	if (a->patch > b->patch) return  1;
	if (a->patch < b->patch) return -1;

	return 0;
}
