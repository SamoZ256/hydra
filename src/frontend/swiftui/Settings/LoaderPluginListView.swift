import SwiftUI
import UniformTypeIdentifiers

struct LoaderPluginListView: View {
    @EnvironmentObject var globalState: GlobalState

    private let pluginType = UTType(filenameExtension: "dylib")!

    @State private var configs: [LoaderPluginConfig] = []

    @State private var isFilePickerPresented = false

    var body: some View {
        Section("Loader Plug-Ins") {
            ForEach(self.$configs, id: \.self) { config in
                HStack {
                    if let plugin = try? HydraLoaderPlugin(path: config.path.wrappedValue) {
                        LoaderPluginView(config: config, plugin: plugin)
                    } else {
                        // TODO: better UI
                        Text("Failed to load \(config.path.wrappedValue)")
                    }
                    DeleteButton(action: {
                        if let index = self.configs.firstIndex(of: config.wrappedValue) {
                            self.configs.remove(at: index)
                        }
                    })
                }
            }

            HStack {
                Spacer()

                Button(action: {
                    isFilePickerPresented.toggle()
                }) {
                    Image(systemName: "plus")
                        .foregroundColor(.gray)
                        .font(.system(size: 16))
                }
                .buttonStyle(PlainButtonStyle())
                .fileImporter(
                    isPresented: $isFilePickerPresented,
                    allowedContentTypes: [pluginType],
                    allowsMultipleSelection: true
                ) { result in
                    switch result {
                    case .success(let fileURLs):
                        for fileURL in fileURLs {
                            do {
                                try registerUrl(fileURL)
                            } catch {
                                print(
                                    "Failed to register URL \(fileURL.path(percentEncoded: false))")
                                continue
                            }
                            self.configs.append(
                                LoaderPluginConfig(
                                    path: fileURL.path(percentEncoded: false), options: [:]))
                        }

                    case .failure(let error):
                        print(error)
                    }
                }
            }
        }
        .onChange(of: self.configs) { _, newValue in
            let loaderPluginsOption = hydraConfigGetLoaderPlugins()
            loaderPluginsOption.resize(to: newValue.count)
            for i in 0..<newValue.count {
                var newHandle = loaderPluginsOption.get(at: i)
                newValue[i].serialize(to: &newHandle)
            }

            // Refresh
            globalState.refreshLoaderPluginManager()
        }
        .onAppear {
            let loaderPluginsOption = hydraConfigGetLoaderPlugins()
            self.configs = []
            for i in 0..<loaderPluginsOption.count {
                self.configs.append(
                    LoaderPluginConfig(handle: loaderPluginsOption.get(at: i)))
            }
        }
    }
}
