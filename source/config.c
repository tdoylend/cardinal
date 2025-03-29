#include <cardinal.h>

#ifdef CAR_NO_DEFAULT
CAR_API bool car_init_default_config(Car_Config *config, void *userdata) { 
	return false;
}
#endif
