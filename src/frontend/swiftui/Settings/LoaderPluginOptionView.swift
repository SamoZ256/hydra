import SwiftUI
import UniformTypeIdentifiers

struct LoaderPluginOptionView: View {
    var config: HydraLoaderPluginOptionConfig
    @Binding var value: String

    @State private var utTypes: [UTType] = []
    @State private var isUTTypesAlertPresented = false
    @State private var isFilePickerPresented = false

    var body: some View {
        VStack(alignment: .leading) {
            HStack {
                Text("\(config.name):")
                    .bold()
                // TODO: all types
                switch config.type {
                case HYDRA_LOADER_PLUGIN_OPTION_TYPE_PATH:
                    HStack {
                        #if os(macOS)
                            let urlBinding = Binding<URL?>(
                                get: {
                                    if value.isEmpty {
                                        return nil
                                    }
                                    return URL(fileURLWithPath: value)
                                },
                                set: { newURL in
                                    if let newURLUnwrapped = newURL {
                                        self.value = newURLUnwrapped.path(percentEncoded: false)
                                    }
                                }
                            )

                            // TODO: don't allow any content type
                            PathControl(
                                url: urlBinding,
                                allowedTypes: nil,
                                placeholderString: config.description
                            )
                            .frame(height: 24)
                        #else
                            Text(self.value)
                        #endif

                        Button(action: {
                            self.isFilePickerPresented = true
                        }) {
                            Text("Choose")
                        }
                        .fileImporter(
                            isPresented: self.$isFilePickerPresented,
                            allowedContentTypes: self.utTypes,
                            allowsMultipleSelection: false
                        ) { result in
                            switch result {
                            case .success(let fileURLs):
                                for fileURL in fileURLs {
                                    do {
                                        try registerUrl(fileURL)
                                    } catch {
                                        print(
                                            "Failed to register URL \(fileURL.path(percentEncoded: false)): \(error)"
                                        )
                                        continue
                                    }
                                    self.value = fileURL.path(percentEncoded: false)
                                }

                            case .failure(let error):
                                print(error)
                            }
                        }
                    }
                    .alert(
                        "Invalid option content types (plug-in error)",
                        isPresented: self.$isUTTypesAlertPresented
                    ) {
                        Button("OK") {
                            self.isUTTypesAlertPresented = false
                        }
                    }
                    .onAppear {
                        do {
                            self.utTypes = try LoaderPluginOptionView.getUTTypes(
                                from: self.config.pathContentTypes.array)
                        } catch {
                            self.isUTTypesAlertPresented = true
                        }
                    }
                default:
                    Text("Not implemented")
                }
            }
        }
    }

    enum ContentTypeError: Error {
        case invalidFormat
        case invalidType
    }

    static func getUTTypes(from contentTypes: [String]) throws -> [UTType] {
        var res: [UTType] = []
        for contentType in contentTypes {
            let components = contentType.components(separatedBy: ".")
            let type = components[0]
            if type == "file" {
                if components.count != 2 {
                    throw ContentTypeError.invalidFormat
                }

                // TODO: allow * and friends
                res.append(UTType(filenameExtension: components[1])!)
            } else if type == "directory" {
                if components.count != 1 {
                    throw ContentTypeError.invalidFormat
                }

                res.append(UTType.directory)
            } else {
                throw ContentTypeError.invalidType
            }
        }

        return res
    }
}
