#!/bin/sh
# Compile the Vulkan custom pipeline shaders to SPIR-V and wrap them as C
# arrays in ../obj/vk_<name>_<stage>.inc. Needs glslangValidator.
cd "$(dirname "$0")"
for src in *.vert *.frag; do
	name=${src%.*}
	stage=${src##*.}
	var=vk_${name}_${stage}_spv
	glslangValidator -V --target-env vulkan1.0 -o "/tmp/$var.spv" "$src" >/dev/null || { glslangValidator -V "$src"; exit 1; }
	{
		echo "static const rw::uint32 $var[] = {"
		od -An -v -tx4 -w16 "/tmp/$var.spv" | sed 's/ \([0-9a-f]\{8\}\)/0x\1, /g; s/^/\t/; s/, $/,/'
		echo "};"
	} > "../obj/$var.inc"
	rm -f "/tmp/$var.spv"
done
