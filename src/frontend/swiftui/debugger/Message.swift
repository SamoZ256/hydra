import SwiftUI

extension HydraLogLevel: Hashable {}

struct Message: Hashable {
    let log_level: HydraLogLevel
    let function: String
    let str: String
    let stack_trace: HydraDebuggerStackTrace

    var style: any ShapeStyle {
        switch self.log_level {
        case HYDRA_LOG_LEVEL_DEBUG:
            return .cyan
        case HYDRA_LOG_LEVEL_INFO:
            return .white
        case HYDRA_LOG_LEVEL_STUB:
            return .purple
        case HYDRA_LOG_LEVEL_WARNING:
            return .yellow
        case HYDRA_LOG_LEVEL_ERROR:
            return .red
        case HYDRA_LOG_LEVEL_FATAL:
            return .red  // TODO: dark red
        default:
            return .white
        }
    }
}
