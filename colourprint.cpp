#include <cstdlib>
#include <iostream>
#include <sstream>

namespace sps {
  namespace colors {
    enum color {
      none    = 0x00,
      black   = 0x01,
      red     = 0x02,
      green   = 0x03,
      yellow  = 0x04,
      blue    = 0x05,
      magenta = 0x06,
      cyan    = 0x07,
      white   = 0x08
    };
  }
  namespace faces {
    enum face {
      normal    = 0x00,
      bold      = 0x01,
      dark      = 0x02,
      uline     = 0x04,
      invert    = 0x07,
      invisible = 0x08,
      cline     = 0x09
    };
  }

  /**
   * Generate string for color codes for std::iostream's
   *
   * @param foreground
   * @param background
   *
   * @return
   *
   * Usage:
   *
   * using namespace sps;
   * std::cout << "These words should be colored [ "
   *           << set_color(colors::red)   << "red "
   *           << set_color(colors::green) << "green "
   *           << set_color(colors::blue)  << "blue"
   *           << set_color() <<  " ]" << std::endl;
   *
   */
  std::string set_color(sps::colors::color foreground = sps::colors::none,
			sps::colors::color background = sps::colors::none) {
    std::stringstream s;
    s << "\033[";
    if (!foreground && ! background){
        s << "0"; // reset colors if no params
    }
    if (foreground) {
        s << 29 + foreground;
        if (background) s << ";";
    }
    if (background) {
        s << 39 + background;
    }
    s << "m";
    return s.str();
  }

  /**
   *
   *
   * @param face
   *
   * @return
   */
  std::string set_face(sps::faces::face face = sps::faces::normal) {
    std::stringstream s;
    s << "\033[";
    if (!face) {
        s << "0"; // reset face
    }
    if (face) {
      s << face;
    }
    s << "m";
    return s.str();
  }
}

#if 0
int main(int agrc, char* argv[])
{
  using namespace sps;
  std::cout << "These words should be colored [ "
        << set_color(colors::red)   << "red "
        << set_color(colors::green) << "green "
        << set_color(colors::blue)  << "blue "
        << set_color(colors::cyan)   << "cyan "
        << set_color(colors::magenta) << "magenta "
        << set_color(colors::yellow)  << "yellow"
        << set_color() <<  " ]" << std::endl;
  return EXIT_SUCCESS;
}
#endif
