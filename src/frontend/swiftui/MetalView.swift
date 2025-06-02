import MetalKit
import SwiftUI

class MetalLayerCoordinator: NSObject {
    private var emulationContext: Binding<UnsafeMutableRawPointer?>

    private var layer: CAMetalLayer? = nil
    private var displayLink: CADisplayLink? = nil

    private var surfaceSet = false

    init(emulationContext: Binding<UnsafeMutableRawPointer?>) {
        self.emulationContext = emulationContext
        super.init()
    }

    deinit {
        self.displayLink?.invalidate()
    }

    func setView(_ view: NSView) {
        self.displayLink?.invalidate()

        self.layer = view.layer as? CAMetalLayer
        self.surfaceSet = false

        // Display link
        //self.displayLink = view.displayLink(target: self, selector: #selector(handleDisplayLink))
        //self.displayLink?.add(to: .main, forMode: .common)

        // TODO: probably not the best way to do this, but it works
        let dispatchQueue = DispatchQueue(label: "present queue", qos: .background)
        dispatchQueue.async {
            while true {
                self.handleDisplayLink()
            }
        }
    }

    @objc func handleDisplayLink() {
        if let emulationContext = self.emulationContext.wrappedValue {
            // Set the surface if its not already set
            if !self.surfaceSet {
                guard let layer = self.layer else {
                    return
                }

                set_layer(emulationContext, layer)
                self.surfaceSet = true
            }

            if hydra_emulation_context_is_running(emulationContext) {
                // Present
                var dtAverageUpdated = false
                hydra_emulation_context_progress_frame(
                    emulationContext, UInt32(self.layer!.drawableSize.width),
                    UInt32(self.layer!.drawableSize.height),
                    &dtAverageUpdated)

                // Update
                if dtAverageUpdated {
                    // TODO
                    /*
                    print(
                        "DT average: \(hydra_emulation_context_get_last_delta_time_average(emulationContext))"
                    )
                    */
                }
            }
        }
    }
}

struct MetalView: NSViewRepresentable {
    @Binding var emulationContext: UnsafeMutableRawPointer?

    func makeNSView(context: Context) -> NSView {
        let view = NSView(frame: .zero)

        let layer = CAMetalLayer()
        layer.displaySyncEnabled = true
        view.layer = layer
        //view.wantsLayer = true

        context.coordinator.setView(view)

        return view
    }

    func updateNSView(_ view: NSView, context: Context) {
        guard let layer = view.layer as? CAMetalLayer else { return }
        layer.frame = view.bounds
        context.coordinator.setView(view)
    }

    func makeCoordinator() -> MetalLayerCoordinator {
        return MetalLayerCoordinator(emulationContext: self.$emulationContext)
    }
}
