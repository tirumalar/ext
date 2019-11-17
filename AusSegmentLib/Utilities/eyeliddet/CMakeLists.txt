ADD_LIBRARY(eyeliddet
    SHARED
    abs.cpp
    bwlabel.cpp
    convn.cpp
    EESMask6.cpp
    EESM_Coverage.cpp
    EESM_emxutil.cpp
    EESM_initialize.cpp
    EESM_rtwutil.cpp
    EESM_terminate.cpp
    GradMag7.cpp
    HiIntensity.cpp
    imfilter.cpp
    imhist.cpp
    LowIntensity.cpp
    make_eyelid_mask.cpp
    median.cpp
    NormIntensity2.cpp
    quickselect.cpp
    rtGetInf.cpp
    rtGetNaN.cpp
    rt_nonfinite.cpp
    sort3.cpp
    sqrt.cpp
    sum.cpp
)

ADD_SUBDIRECTORY(test)

INSTALL(TARGETS eyeliddet LIBRARY DESTINATION lib)
