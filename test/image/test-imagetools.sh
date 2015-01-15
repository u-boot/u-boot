#!/bin/bash
#
# Written by Guilherme Maciel Ferreira <guilherme.maciel.ferreira@gmail.com>
#
# Sanity check for mkimage and dumpimage tools
#
# SPDX-License-Identifier:	GPL-2.0+
#
# To run this:
#
# make O=sandbox sandbox_config
# make O=sandbox
# ./test/image/test-imagetools.sh

BASEDIR=sandbox
SRCDIR=${BASEDIR}/boot
IMAGE_NAME="v1.0-test"
IMAGE_MULTI=linux.img
DATAFILE0=vmlinuz
DATAFILE1=initrd.img
DATAFILE2=System.map
DATAFILES="${DATAFILE0} ${DATAFILE1} ${DATAFILE2}"
TEST_OUT=test_output
MKIMAGE=${BASEDIR}/tools/mkimage
DUMPIMAGE=${BASEDIR}/tools/dumpimage
MKIMAGE_LIST=mkimage.list
DUMPIMAGE_LIST=dumpimage.list

# Remove all the files we created
cleanup()
{
	local file

	for file in ${DATAFILES}; do
		rm -f ${file} ${SRCDIR}/${file}
	done
	rm -f ${IMAGE_MULTI} ${DUMPIMAGE_LIST} ${MKIMAGE_LIST} ${TEST_OUT}
	rmdir ${SRCDIR}
}

# Check that two files are the same
assert_equal()
{
	if ! diff -u $1 $2; then
		echo "Failed."
		cleanup
		exit 1
	fi
}

# Create some test files
create_files()
{
	local file

	mkdir -p ${SRCDIR}
	for file in ${DATAFILES}; do
		head -c $RANDOM /dev/urandom >${SRCDIR}/${file}
	done
}

# Run a command, echoing it first
do_cmd()
{
	local cmd="$@"

	echo "# ${cmd}"
	${cmd} 2>&1
}

# Run a command, redirecting output
# Args:
#    redirect_file
#    command...
do_cmd_redir()
{
	local redir="$1"
	shift
	local cmd="$@"

	echo "# ${cmd}"
	${cmd} >${redir}
}

# Write files into an multi-file image
create_multi_image()
{
	local files="${SRCDIR}/${DATAFILE0}:${SRCDIR}/${DATAFILE1}"
	files+=":${SRCDIR}/${DATAFILE2}"

	echo -e "\nBuilding multi-file image..."
	do_cmd ${MKIMAGE} -A x86 -O linux -T multi -n \"${IMAGE_NAME}\" \
		-d ${files} ${IMAGE_MULTI}
	echo "done."
}

# Extract files from an multi-file image
extract_multi_image()
{
	echo -e "\nExtracting multi-file image contents..."
	do_cmd ${DUMPIMAGE} -T multi -i ${IMAGE_MULTI} -p 0 ${DATAFILE0}
	do_cmd ${DUMPIMAGE} -T multi -i ${IMAGE_MULTI} -p 1 ${DATAFILE1}
	do_cmd ${DUMPIMAGE} -T multi -i ${IMAGE_MULTI} -p 2 ${DATAFILE2}
	do_cmd ${DUMPIMAGE} -T multi -i ${IMAGE_MULTI} -p 2 ${DATAFILE2} -o ${TEST_OUT}
	echo "done."
}

# List the contents of a file
# Args:
#    image filename
list_image()
{
	local image="$1"

	echo -e "\nListing image contents..."
	do_cmd_redir ${MKIMAGE_LIST} ${MKIMAGE} -l ${image}
	do_cmd_redir ${DUMPIMAGE_LIST} ${DUMPIMAGE} -l ${image}
	echo "done."
}

main()
{
	local file

	create_files

	# Compress and extract multi-file images, compare the result
	create_multi_image
	extract_multi_image
	for file in ${DATAFILES}; do
		assert_equal ${file} ${SRCDIR}/${file}
	done
	assert_equal ${TEST_OUT} ${DATAFILE2}

	# List contents of multi-file image and compares output from tools
	list_image ${IMAGE_MULTI}
	assert_equal ${DUMPIMAGE_LIST} ${MKIMAGE_LIST}

	# Remove files created
	cleanup

	echo "Tests passed."
}

main
