#ifndef __LOGOPRISM_CONFIG_YAML_PARSER_HPP__
#define __LOGOPRISM_CONFIG_YAML_PARSER_HPP__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/file_parser_error.hpp>

#include <yaml-cpp/yaml.h>
#include <fstream>

/**
 * YAML parsing code for boost::property_tree, taken from somewhere on the internet
 */
namespace logoprism {
  namespace yaml {

    struct yaml_parser_error : boost::property_tree::file_parser_error {
      yaml_parser_error(std::string const& message, std::string const& filename, size_t line) :
        file_parser_error(message, filename, line)
      {}
    };

    template< class Ptree >
    void read_yaml_helper(YAML::Node const& node, std::string const& key, Ptree& ptree, bool root=false) {
      if (node.Type() == YAML::NodeType::Map) {
        Ptree* next;
        if (root)
          next = &ptree;
        else if (key != "")
          next = &ptree.put_child(key, ptree.get_child(key, Ptree()));
        else
          next = &ptree.push_back(std::make_pair(key, Ptree()))->second;

        for (YAML::Iterator child = node.begin(), end = node.end(); child != end; ++child) {
          std::string      key;
          child.first() >> key;
          read_yaml_helper(child.second(), key, *next);
        }

      } else if (node.Type() == YAML::NodeType::Sequence) {
        Ptree* next;
        if (root)
          next = &ptree;
        else if (key != "")
          next = &ptree.put_child(key, ptree.get_child(key, Ptree()));
        else
          next = &ptree.push_back(std::make_pair(key, Ptree()))->second;

        for (auto const& child : node) {
          read_yaml_helper(child, "", *next);
        }

      } else {
        if (key != "")
          ptree.put(key, node.to< std::string >());
        else
          ptree.push_back(std::make_pair(key, Ptree(node.to< std::string >())));
      }

    } // read_yaml_helper

    template< class Ptree >
    void read_yaml_internal(std::basic_istream< typename Ptree::key_type::value_type >& stream, Ptree& ptree, std::string const&) {
      YAML::Parser parser(stream);
      YAML::Node   document;

      while (parser.GetNextDocument(document)) {
        read_yaml_helper(document, "", ptree, true);
      }
    }

    template< class Ptree >
    void read_yaml(std::basic_istream< typename Ptree::key_type::value_type >& stream, Ptree& ptree) {
      read_yaml_internal(stream, ptree, std::string());
    }

    template< class Ptree >
    void read_yaml(std::string const& filename, Ptree& ptree, std::locale const& loc=std::locale()) {
      std::basic_ifstream< typename Ptree::key_type::value_type >
      stream(filename.c_str());

      if (!stream)
        BOOST_PROPERTY_TREE_THROW(yaml_parser_error("cannot open file", filename, 0));
      stream.imbue(loc);

      read_yaml_internal(stream, ptree, filename);
    }

    template< class Ptree >
    void write_yaml_helper(YAML::Emitter& stream, Ptree const& ptree) {
      typedef typename Ptree::key_type::value_type    char_type;
      typedef typename std::basic_string< char_type > string_type;

      if (ptree.empty()) {

#define YAML_TRY_GET(type_m)                          \
  do {                                                \
    try {                                             \
      stream << ptree.template get_value< type_m >(); \
      return;                                         \
    } catch (boost::exception const& e) {             \
    }                                                 \
  } while (0)

        YAML_TRY_GET(uint64_t);
        YAML_TRY_GET(int64_t);
        YAML_TRY_GET(double);
        YAML_TRY_GET(bool);

#undef YAML_TRY_GET

        stream << YAML::SingleQuoted << ptree.template get_value< string_type >();

      } else if (ptree.count(string_type()) == ptree.size()) {
        if (std::count_if(ptree.begin(), ptree.end(), [](typename Ptree::value_type const& node) { return node.second.empty(); }) == ptree.size())
          stream << YAML::Flow;

        stream << YAML::BeginSeq;
        for (auto const& node : ptree) {
          write_yaml_helper(stream, node.second);
        }
        stream << YAML::EndSeq;

      } else {
        stream << YAML::BeginMap;
        for (auto const& node : ptree) {
          stream << YAML::Key << node.first;
          stream << YAML::Value;
          write_yaml_helper(stream, node.second);
        }
        stream << YAML::EndMap;

      }
    } // write_yaml_helper

    template< class Ptree >
    void write_yaml_internal(std::basic_ostream< typename Ptree::key_type::value_type >& stream, Ptree const& ptree, std::string const& filename) {
      YAML::Emitter out;

      out << YAML::BeginDoc;
      write_yaml_helper(out, ptree);
      out << YAML::EndDoc;
      stream << out.c_str();

      if (!stream.good())
        BOOST_PROPERTY_TREE_THROW(yaml_parser_error("write error", filename, 0));
    }

    template< class Ptree >
    void write_yaml(std::basic_ostream< typename Ptree::key_type::value_type >& stream, Ptree const& ptree) {
      write_yaml_internal(stream, ptree, std::string());
    }

    template< class Ptree >
    void write_yaml(std::string const& filename, Ptree const& ptree, std::locale const& loc=std::locale()) {
      std::basic_ofstream< typename Ptree::key_type::value_type > stream(filename.c_str());

      if (!stream)
        BOOST_PROPERTY_TREE_THROW(yaml_parser_error("cannot open file", filename, 0));
      stream.imbue(loc);

      write_yaml_internal(stream, ptree, filename);
    }

  }
}

#endif // ifndef __LOGOPRISM_CONFIG_YAML_PARSER_HPP__ const
