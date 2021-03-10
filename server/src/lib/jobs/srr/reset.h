#pragma once

#include <fty_srr_dto.h>

namespace fty::job::srr {

dto::srr::ResetResponse reset(const dto::srr::ResetQuery& query);

}
