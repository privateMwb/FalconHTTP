vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO privateMwb/FalconHTTP
    REF a903e0bf7f9dde22734ee9511f1e216bd6120233
    SHA512 be564aa2e6da228036cf4ca841f4af2100d15f498ec04057db6a860847d83013cbd2dea47d1a7d78544d4a5ab36325403025a91c39a9ecf542aacd276a17fb9e
)

# GitHub archive tarballs never include submodule content, so
# FalconHTTP's internal libraries under libs/internal/ are fetched
# separately here, each pinned to the exact commit the submodule
# points at, then copied into place.
vcpkg_from_github(
    OUT_SOURCE_PATH FUNCTIONPRO_SOURCE_PATH
    REPO privateMwb/FunctionPro
    REF 1c63b9378f662b9cb723cf2405f66773ab502dbe
    SHA512 67e982701cbd36e3ad3041ffeeb650fbb6c5314246040e4e0cb1aecd286b52756594f2c0ba2b472ee9280c0bec2ab674683a086e5b32422911f336d825aa3597
)
vcpkg_from_github(
    OUT_SOURCE_PATH HASHMAPPRO_SOURCE_PATH
    REPO privateMwb/HashMapPro
    REF 497b996dfa4721136a4c89edefd36e6a0dfe1d09
    SHA512 3d696060c666b9ea9afa02e9394f637967851cf167d864ec2699858b02370fca52ad8c796552be726d0534373fa828aec0879aaa9ba17d9692b902c199c8eba9
)
vcpkg_from_github(
    OUT_SOURCE_PATH JSONPARSER_SOURCE_PATH
    REPO privateMwb/JsonParser
    REF e4668f451d9bfd14d09f55bb4613e40be893e30e
    SHA512 421980c1f8f5b327a8b8d61ba0bb9eb860081ee713daa7a9c866de260a19752e8affba22cffd28c689f791c4ce53650000acfe9efe187e976a22026b009859a4
)
vcpkg_from_github(
    OUT_SOURCE_PATH LRUCACHE_SOURCE_PATH
    REPO privateMwb/LRUCache
    REF b82b2f00aaafdc205693760ec0e0e191752b95b6
    SHA512 c8d606e2ee9814b6abdfeae00d12ac935e04a06aa5bd79efd04ebb065e54a06e0df95f84ca00e19624d91a4d95c70154ad0a0b0b9caa5713fed513eb7574dfde
)
vcpkg_from_github(
    OUT_SOURCE_PATH THREADPOOLPRO_SOURCE_PATH
    REPO privateMwb/ThreadPoolPro
    REF 260864a7ec3cb2c5135cd2effe402b20974c04f0
    SHA512 6d6314f614aa29184079d89727eefa730a9298452329a047a412c71ede467f15398d06d4409e8776dc7b76eae384cfaf16b8e0a0f220afc9f6e4424c4150d285
)
vcpkg_from_github(
    OUT_SOURCE_PATH VECTORPRO_SOURCE_PATH
    REPO privateMwb/VectorPro
    REF 26407a59ecd6fefe69565c980b8de49332e469e8
    SHA512 969c97bfad58f94a75ba35c8723812badad112d5b304e73b555715ad61657055a7c41c6568fadeb5e8a21bca27215435abef28e6a1221ff14f6d2244ab3ca081
)

foreach(SUBMODULE_NAME FunctionPro HashMapPro JsonParser LRUCache ThreadPoolPro VectorPro)
    file(REMOVE_RECURSE "${SOURCE_PATH}/libs/internal/${SUBMODULE_NAME}")
endforeach()
file(RENAME "${FUNCTIONPRO_SOURCE_PATH}" "${SOURCE_PATH}/libs/internal/FunctionPro")
file(RENAME "${HASHMAPPRO_SOURCE_PATH}" "${SOURCE_PATH}/libs/internal/HashMapPro")
file(RENAME "${JSONPARSER_SOURCE_PATH}" "${SOURCE_PATH}/libs/internal/JsonParser")
file(RENAME "${LRUCACHE_SOURCE_PATH}" "${SOURCE_PATH}/libs/internal/LRUCache")
file(RENAME "${THREADPOOLPRO_SOURCE_PATH}" "${SOURCE_PATH}/libs/internal/ThreadPoolPro")
file(RENAME "${VECTORPRO_SOURCE_PATH}" "${SOURCE_PATH}/libs/internal/VectorPro")

set(VCPKG_PORT_NAME FalconHTTP)

# Consumers only need the library itself, not the tests, benchmarks,
# regression tools, or examples.
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_BENCHMARKS=OFF
        -DBUILD_REGRESSION=OFF
        -DBUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME ${VCPKG_PORT_NAME}
    CONFIG_PATH lib/cmake/${VCPKG_PORT_NAME}
)

# This library is compiled (not header-only), so debug binaries are
# real and must be kept — only the duplicate debug/include headers
# are removed.
file(
    REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
)

vcpkg_install_copyright(
    FILE_LIST "${SOURCE_PATH}/LICENSE"
)
