#ifndef __LOGOPRISM_CONFIG_CONFIG_HPP__
#define __LOGOPRISM_CONFIG_CONFIG_HPP__

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

namespace logoprism {
  namespace config {

    /**
     * Initializes the program configuration by loading the corresponding configuration file and parsing the command-line arguments.
     *
     * @param program   the name of the program that started (used to find a configuration file named <program>.conf)
     * @param arguments the arguments from the command line
     */
    void init(std::string const& program, std::vector< std::string > const& arguments);

    typedef boost::property_tree::ptree tree;

    /**
     * String wrapper that can implicitely be converted to any type using boost::lexical_cast, for easy configuration value retrieval.
     */
    struct leaf : std::string {
      explicit leaf(std::string const& value) : std::string(value) {}
      template< typename T > operator T() const { return boost::lexical_cast< T >(static_cast< std::string >(*this)); }

      /** special case for bool to accept text form */
      operator bool() const { return *this == "true" ? true : *this == "false" ? false : boost::lexical_cast< bool >(static_cast< std::string >(*this)); }
    };

    extern config::tree instance;

    /**
     * Get a configuration value, or return the given default_value if not found.
     *
     * @param  key           the configuration key to look for
     * @param  default_value the default value to return if the key is not found
     * @return               the value corresponding to the given key, or default_value
     */
    template< typename T >
    static inline leaf get(std::string const& key, T const& default_value)
    { return config::leaf(instance.get(key, boost::lexical_cast< std::string >(default_value))); }

    /**
     * Get a configuration value, or return an empty value if not found.
     *
     * @param  key the configuration key to look for
     * @return     the value corresponding to the given key, or an empty value
     */
    static inline leaf get(std::string const& key)
    { return config::leaf(instance.get< std::string >(key)); }

    /**
     * Get a configuration sub-tree for the given configuration key
     * @param  key the configuration key to look for
     * @return     the sub-tree corresponding to the given key
     */
    static inline tree get_child(std::string const& key)
    { return instance.get_child(key); }

  }
}

#endif // ifndef __LOGOPRISM_CONFIG_CONFIG_HPP__
