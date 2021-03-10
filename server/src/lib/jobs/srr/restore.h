#pragma once

#include <fty_srr_dto.h>

namespace fty::job::srr {

dto::srr::RestoreResponse restore(const dto::srr::RestoreQuery& query);

}
