# Run test coverage
make CFG=coverage
bin/coverage/main -t
make gcov
xdg-open cov/index.html
