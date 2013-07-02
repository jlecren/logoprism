#include "logoprism/config/config.hpp"
#include "logoprism/data/types.hpp"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <vector>

#include "logoprism/config/yaml_parser.hpp"

namespace logoprism {
  namespace config {

    config::tree instance;

    namespace {
      namespace bpo = boost::program_options;
      namespace bfs = boost::filesystem;

      /**
       * An helper structure to put values in the configuration tree at a given key, whenever the
       * command-line parsing library notifies the callbacks.
       */
      template< typename T >
      struct option {
        option(std::string const& key) : key(key) {}

        void operator()(T const& value) const { config::instance.put(this->key, value); }

        void operator()(std::vector< T > const& values) const {
          auto& child = config::instance.put_child(this->key, config::tree());

          for (auto const& v : values) { child.push_back(std::make_pair("", config::tree(v))); }
        }

        operator bpo::typed_value< T >*() { return bpo::value< T >()->notifier(*this); }
        bpo::typed_value< T >* operator->() { return static_cast< bpo::typed_value< T >* >(*this); }

        std::string const key;
      };

    }

    void init(std::string const& program, std::vector< std::string > const& arguments) {
      auto const application_path = bfs::canonical(bfs::path(program).remove_filename());
      auto const application_name = bfs::path(program).stem().string();

      auto const config_name = application_name + ".conf";
      auto const config_path = application_path / config_name;

      // save the program and arguments in some predefined configuration keys
      option< std::string >("command.program")(program);
      option< std::string >("command.arguments")(arguments);

      // if the configuration file exists, load it
      if (bfs::exists(config_path))
        yaml::read_yaml(config_path.string(), config::instance);

      // if there's no command line arguments, nothing to do
      if (arguments.empty())
        return;

      bpo::options_description generic_options("Generic options");
      generic_options.add_options()
        ("version,v", "print version string")
        ("help", "produce help message")
        ("config,c", option< std::string >("config.file"), "tree file");

      bpo::options_description logoprism_options("LogO'Prism options");
      logoprism_options.add_options()
        ("input-file,i", option< std::string >("input.file"), "input files")
        ("input-format", option< std::string >("input.format"), "input format")
        ("input-speed", option< double >("input.speed")->default_value(1.0), "input speed")
        ("input-keepalive", option< double >("input.keepalive")->default_value(5.0), "input keep-alive time")
        ("display-width,w", option< size_t >("display.width")->default_value(1024), "display width")
        ("display-height,h", option< size_t >("display.height")->default_value(560), "display height")
        ("display-fullscreen,f", option< bool >("display.fullscreen")->default_value(false)->zero_tokens(), "run full screen")
        ("display-multisampling", option< bool >("display.multisampling")->default_value(false)->zero_tokens(), "use multisampling")
        ("output-video,o", option< bool >("output.video")->default_value(false)->zero_tokens(), "encode video")
        ("output-framerate", option< size_t >("output.framerate"), "output frame rate (fps)")
        ("output-pipeline", option< std::string >("output.pipeline"), "output gstreamer pipeline");

      bpo::options_description command_line_options;
      command_line_options.add(generic_options).add(logoprism_options);

      auto const& options = bpo::command_line_parser(arguments).options(command_line_options).allow_unregistered().run();

      // parse the options and notify the callbacks
      bpo::variables_map vm;
      bpo::store(options, vm);
      bpo::notify(vm);

      // if some more config file was passed from the command-line, load it
      auto const& config_file = config::instance.get("config.file", "");
      if (!config_file.empty())
        yaml::read_yaml(config_file, config::instance);

      // if the user wants some help, print it and exit
      if (vm.count("help")) {
        std::cout << command_line_options << std::endl;
        std::exit(0);
      }
    } // init

  }
}
