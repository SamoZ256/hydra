#pragma once

namespace hydra::frontend::native::cocoa {

class Native {
  public:
    Native();
    ~Native();

    bool showInputTextDialog(const std::string& header_text,
                             const std::string& sub_text,
                             const std::string& guide_text,
                             std::string& out_text);

  private:
    id text_input_delegate;
};

} // namespace hydra::frontend::native::cocoa
