#add sub directory
ADD_SUBDIRECTORY(codec)
ADD_SUBDIRECTORY(format)

#av files
AUX_SOURCE_DIRECTORY(. DIR_STU_AV_SRC)

#build av
ADD_LIBRARY(av ${DIR_STU_AV_SRC})
