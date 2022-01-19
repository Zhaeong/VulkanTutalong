#include "main.hpp"

int main() {
  std::cout << "Starting App Tho\n";
  ve::FirstApp app;

  try {
    app.run();
  } catch (const std::exception &e) {

    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}