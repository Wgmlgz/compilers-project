#!/bin/bash

# Colors for better output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Ensure the compiler is built
echo -e "${YELLOW}Building the compiler...${NC}"
make clean && make build

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed. Exiting.${NC}"
    exit 1
fi

echo -e "\n${GREEN}Build successful. Running tests...${NC}\n"

# Function to run a test
run_test() {
    test_file=$1
    expect_failure=$2
    
    echo -e "${YELLOW}Running: ${test_file}${NC}"
    
    if [ "$expect_failure" = "true" ]; then
        # We expect this test to fail
        ./out/js < "$test_file" > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo -e "${GREEN}Test failed as expected. ✓${NC}"
            # Run again to see the error message
            echo -e "${YELLOW}Error details:${NC}"
            ./out/js < "$test_file" 2>&1 | head -n 10
            echo "..."
        else
            echo -e "${RED}Test unexpectedly passed. ✗${NC}"
            return 1
        fi
    else
        # We expect this test to pass
        ./out/js < "$test_file"
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}Test passed. ✓${NC}"
        else
            echo -e "${RED}Test failed. ✗${NC}"
            return 1
        fi
    fi
    
    echo -e "--------------------------------------------\n"
    return 0
}

# Run regular ArrowRust tests
echo -e "${YELLOW}Running ArrowRust test files (should pass):${NC}\n"

failures=0
for test_file in tests/legit/*.rs; do
    run_test "$test_file" "false"
    failures=$((failures + $?))
done

# Run bad examples (these should fail with detailed errors)
echo -e "\n${YELLOW}Running bad example test files (should fail with detailed errors):${NC}\n"

for test_file in tests/bad_examples/*.rs; do
    run_test "$test_file" "true"
    # We don't count failures here as these are supposed to fail
done

# Print summary
if [ $failures -eq 0 ]; then
    echo -e "\n${GREEN}All expected tests passed successfully!${NC}"
else
    echo -e "\n${RED}${failures} tests failed that were expected to pass.${NC}"
    exit 1
fi 