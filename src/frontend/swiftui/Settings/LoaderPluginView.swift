import SwiftUI

struct LoaderPluginConfig: Equatable, Hashable {
    var path: String
    var options: [String: String]

    init(path: String, options: [String: String]) {
        self.path = path
        self.options = options
    }

    init(handle: HydraLoaderPluginConfig) {
        self.path = handle.path
        self.options = [:]

        let options = handle.options
        for i in 0..<options.count {
            let key = options.getKey(at: i)
            let value = options.getValue(at: i)
            self.options[key] = value
        }
    }

    func serialize(to handle: inout HydraLoaderPluginConfig) {
        handle.path = self.path
        for (key, value) in self.options {
            handle.options.set(byKey: key, value: value)
        }
    }
}

struct LoaderPluginView: View {
    @Binding var config: LoaderPluginConfig
    var plugin: HydraLoaderPlugin

    @State private var name = ""
    @State private var displayVersion = ""
    @State private var supportedFormats: [String] = []
    @State private var optionConfigs: [HydraLoaderPluginOptionConfig] = []

    @State private var showEditor = false

    var body: some View {
        HStack {
            VStack(alignment: .leading) {
                Text(self.name)
                    .bold()
                Text(self.displayVersion)
                Text("Supported formats: \(self.supportedFormats.joined(separator: ", "))")
            }

            Spacer()

            Button(action: {
                self.showEditor = true
            }) {
                Image(systemName: "pencil")
            }
            .buttonStyle(BorderlessButtonStyle())
        }
        .sheet(isPresented: self.$showEditor) {
            VStack(alignment: .leading) {
                Text(self.name)
                Section {
                    ForEach(self.optionConfigs, id: \.self) { config in
                        LoaderPluginOptionView(
                            config: config,
                            value: Binding(
                                get: { self.config.options[config.name] ?? "" },
                                set: {
                                    // HACK
                                    var copy = self.config.options
                                    copy[config.name] = $0
                                    self.config.options = copy
                                }
                            ))
                    }
                }
            }
            .padding()
        }
        .onAppear {
            self.name = plugin.name
            self.displayVersion = plugin.displayVersion
            self.supportedFormats = []
            for i in 0..<plugin.getSupportedFormatCount() {
                self.supportedFormats.append(plugin.getSupportedFormat(at: i))
            }
            self.optionConfigs = []
            for i in 0..<plugin.getOptionConfigCount() {
                self.optionConfigs.append(plugin.getOptionConfig(at: i))
            }
        }
    }
}
