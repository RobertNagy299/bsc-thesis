#include "../public_api.hpp"

namespace LoggerService {

bool is_silent_mode = false;

void setSilentMode(bool silent) { is_silent_mode = silent; }

} // namespace LoggerService