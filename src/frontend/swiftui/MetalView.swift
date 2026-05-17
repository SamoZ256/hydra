import MetalKit
import SwiftUI

class MetalLayerCoordinator: NSObject {
    @Binding var system: HydraSystem?
    @Binding var fps: Int

    private var layer: CAMetalLayer? = nil
    private var displayLink: CADisplayLink? = nil
    private var surfaceSet = false

    init(system: Binding<HydraSystem?>, fps: Binding<Int>) {
        self._system = system
        self._fps = fps
        super.init()
    }

    deinit {
        self.displayLink?.invalidate()
    }

    #if os(macOS)
        func setView(_ view: NSView) {
            self.displayLink?.invalidate()
            self.layer = view.layer as? CAMetalLayer
            self.surfaceSet = false

            // Display link
            self.displayLink = view.displayLink(
                target: self, selector: #selector(handleDisplayLink))
            self.displayLink?.add(to: .main, forMode: .common)
        }
    #else
        func setView(_ view: UIView) {
            self.displayLink?.invalidate()
            self.layer = view.layer as? CAMetalLayer
            self.surfaceSet = false

            // Display link
            self.displayLink = CADisplayLink(target: self, selector: #selector(handleDisplayLink))
            self.displayLink?.add(to: .main, forMode: .commonModes)
        }
    #endif

    @objc func handleDisplayLink() {
        if let system = system {
            // Set the surface if its not already set
            if !self.surfaceSet {
                guard let layer = self.layer else {
                    return
                }
                system.surface = Unmanaged.passUnretained(layer).toOpaque()
                self.surfaceSet = true
            }

            // Present
            var dtAverageUpdated = false
            system.progressFrame(
                width: UInt32(self.layer!.drawableSize.width),
                height: UInt32(self.layer!.drawableSize.height),
                dtAverageUpdated: &dtAverageUpdated)

            // Update
            if dtAverageUpdated {
                let dt = system.getLastDeltaTimeAverage()
                if dt != 0.0 {
                    fps = Int((1.0 / dt).rounded())
                } else {
                    fps = 0
                }
            }
        }
    }
}

#if os(macOS)
    struct MetalView: NSViewRepresentable {
        @EnvironmentObject var globalState: GlobalState

        @Binding var fps: Int

        func makeNSView(context: Context) -> NSView {
            let view = NSView(frame: .zero)
            let layer = CAMetalLayer()

            layer.displaySyncEnabled = true
            layer.contentsScale = NSScreen.main?.backingScaleFactor ?? 2.0
            view.layer = layer
            view.wantsLayer = true
            context.coordinator.setView(view)

            return view
        }

        func updateNSView(_ view: NSView, context: Context) {
            guard let layer = view.layer as? CAMetalLayer else { return }
            layer.frame = view.bounds
            context.coordinator.setView(view)
        }

        func makeCoordinator() -> MetalLayerCoordinator {
            return MetalLayerCoordinator(
                system: $globalState.system, fps: self.$fps)
        }
    }
#else
    class MetalBackedView: UIView {
        override class var layerClass: AnyClass {
            return CAMetalLayer.self
        }

        override func didMoveToWindow() {
            super.didMoveToWindow()
            if let metalLayer = layer as? CAMetalLayer {
                metalLayer.contentsScale = window?.screen.scale ?? UIScreen.main.scale
            }
        }
    }

    struct MetalView: UIViewRepresentable {
        @EnvironmentObject var globalState: GlobalState

        @Binding var fps: Int

        func makeUIView(context: Context) -> UIView {
            let view = MetalBackedView(frame: .zero)
            context.coordinator.setView(view)
            return view
        }

        func updateUIView(_ view: UIView, context: Context) {
            if let layer = view.layer.sublayers?.first as? CAMetalLayer {
                layer.frame = view.bounds
                layer.contentsScale = view.window?.screen.scale ?? UIScreen.main.scale
            }
            context.coordinator.setView(view)
        }

        func makeCoordinator() -> MetalLayerCoordinator {
            return MetalLayerCoordinator(system: $globalState.system, fps: $fps)
        }
    }
#endif
