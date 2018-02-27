######################################################################
# Copyright (C) 2017 N7 Space sp. z o. o.
# Contact: http://n7space.com
#
# This file is part of ASN.1/ACN Plugin for QtCreator.
#
# Plugin was developed under a programme and funded by
# European Space Agency.
#
# This Plugin is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This Plugin is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#######################################################################

defineReplace(createFileNames) {
    allFiles = $${1}

    for(file, allFiles) {
        fileBaseName = $$basename(file)
        splitted = $$split(fileBaseName, ".")
        extension = $$last(splitted)

        equals(extension, asn)|equals(extension, asn1) {
            fileNames += $$first(splitted)
        }
    }

    return($$fileNames)
}

defineReplace(createEmptyFiles) {
    files = $${1}
    contents = "File will be generated by asn1scc"

    for(file, files) {
        !exists($$file) {
            write_file($$file, $$contents)
        }
    }
    return($$files)
}

defineReplace(createHeadersList) {
    headersNames = $${1}

    for(name, headersNames) {
        header = $$clean_path($${ASN1SCC_PRODUCTS_DIR}/$${name}.h)
        headers += $$header
    }

    return($$createEmptyFiles($$headers))
}

defineReplace(createSourcesList) {
    sourcesNames = $${1}

    for(name, sourcesNames) {
        source = $$clean_path($${ASN1SCC_PRODUCTS_DIR}/$${name}.c)
        sources += $$source
    }

    return($$createEmptyFiles($$sources))
}

names = $$createFileNames($$DISTFILES)

!isEmpty(names) {
    PERSISTENT_HEADERS = $${ASN1SCC_PRODUCTS_DIR}/asn1crt.h
    PERSISTENT_SOURCES = $${ASN1SCC_PRODUCTS_DIR}/asn1crt.c $${ASN1SCC_PRODUCTS_DIR}/real.c

    contains(ASN1SCC_GENERATION_OPTIONS, -ACN) {
        PERSISTENT_SOURCES += $${ASN1SCC_PRODUCTS_DIR}/acn.c
    }
}

SOURCES += $$createSourcesList($$names) $$createEmptyFiles($$PERSISTENT_SOURCES)
HEADERS += $$createHeadersList($$names) $$createEmptyFiles($$PERSISTENT_HEADERS)

INCLUDEPATH += $$ASN1SCC_PRODUCTS_DIR
