build_lib_example(
  NAME olsb-manet
  SOURCE_FILES olsb-manet.cc
  LIBRARIES_TO_LINK
    ${libwifi}
    ${libinternet}
    ${libolsb}
    ${libapplications}
)

build_lib_example(
  NAME olsb_two_points
  SOURCE_FILES olsb_two_points.cc
  LIBRARIES_TO_LINK
    ${libwifi}
    ${libinternet}
    ${libolsb}
    ${libapplications}
)

build_lib_example(
  NAME olsb_run_comparison
  SOURCE_FILES olsb_run_comparison.cc
  LIBRARIES_TO_LINK
    ${libwifi}
    ${libinternet}
    ${libolsb}
    ${libapplications}
)
