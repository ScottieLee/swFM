//Scott Lee, 2013
#include "model.h"

model::model():model_spec(*(new usrp_subdev_spec(1,0)))
{
}
int  model::model_hw_decim = 16;
