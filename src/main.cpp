#include "logoprism/utils/signals.hpp"
#include "logoprism/config/config.hpp"
#include "logoprism/logoprism.hpp"

#include <boost/filesystem/path.hpp>

#ifdef _WIN32
# include <boost/locale/encoding_utf.hpp>
#endif // ifdef _WIN32

namespace logoprism {

  static void main(std::string const& program, std::vector< std::string > const& arguments) {
    std::locale::global(std::locale::classic());
    std::cin.imbue(std::locale());
    std::cout.imbue(std::locale());
    std::cerr.imbue(std::locale());
    std::clog.imbue(std::locale());
    boost::filesystem::path::imbue(std::locale());

    utils::signals::install();

    config::init(program, arguments);

    logoprism application;

    application.run();
  }

}

#ifdef _WIN32
# include <windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  auto const to_utf8  = [](std::wstring const& utf16) { return boost::locale::conv::utf_to_utf< char >(utf16); };
  auto const to_utf16 = [](std::string const& utf8) { return boost::locale::conv::utf_to_utf< wchar_t >(utf8); };

  try {
    int     argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    std::string const          program = to_utf8(argv[0]);
    std::vector< std::string > arguments;
    std::transform(argv + 1, argv + argc, std::back_inserter(arguments), to_utf8);

    logoprism::main(program, arguments);
  } catch (std::exception const& e) {
    MessageBoxW(NULL, to_utf16(std::string("Exception raised: \n") + e.what()).c_str(), to_utf16("Exception raised !").c_str(), MB_OK | MB_ICONERROR);
  }
}

#else // ifdef _WIN32

extern "C" int main(int argc, char const* const* argv) {
  std::string const          program = argv[0];
  std::vector< std::string > arguments;
  std::copy(argv + 1, argv + argc, std::back_inserter(arguments));

  logoprism::main(program, arguments);
}

#endif // ifdef _WIN32
