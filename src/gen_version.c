/**
 * \file gen_version.c
 * \brief generate glc version header based on git status
 * \author Pyry Haulos <pyry.haulos@gmail.com>
 * \date 2007-2008
 * For conditions of distribution and use, see copyright notice in glc.h
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SHORTVER_LEN 7

char *extract_version(const char *basever, const char *path)
{
	int result;

	size_t basever_len = strlen(basever);

	const char *cmd_format = "git --git-dir=\"%s\" rev-parse HEAD 2> /dev/null";
	size_t cmd_len = strlen(path) + strlen(cmd_format) + 1;

	char *version, *cmd = malloc(sizeof(char) * cmd_len);

	snprintf(cmd, cmd_len, cmd_format, path);

	FILE *in = popen(cmd, "r");
	if (!in) {
		free(cmd);
		return NULL;
	}
	free(cmd);

	version = malloc(sizeof(char) * (SHORTVER_LEN + 1 + basever_len));
	memcpy(version, basever, basever_len);
	result = fread(&version[basever_len], 1, SHORTVER_LEN, in);

	if (result != SHORTVER_LEN) {
		free(version);
		return NULL;
	}

	version[basever_len + SHORTVER_LEN] = '\0';

	pclose(in);

	return version;
}

int write_version(const char *filename, const char *version)
{
	int result;
	FILE *out = fopen(filename, "w");

	if (!out) {
		fprintf(stderr, "Can't open %s: %s (%d)\n", filename, strerror(errno), errno);
		return errno;
	}

	result = fprintf(out, "#ifndef _VERSION_H\n" \
			   "#define _VERSION_H\n" \
			   "/* generated by %s (compiled %s %s) */\n" \
			   "#define GLC_VERSION \"%s\"\n" \
			   "#endif\n", __FILE__, __DATE__, __TIME__, version);
	if (result < 0) {
		result = ferror(out);
		fprintf(stderr, "Can't write version information: %s (%d)\n", strerror(result), result);
		return result;
	}

	fclose(out);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr, "%s [target] [version] [git directory]\n", argv[0]);
		return EXIT_FAILURE;
	}

	const char *target = argv[1];
	const char *version = argv[2];
	const char *git_dir = argv[3];
	char *git_version = extract_version(version, git_dir);

	if (git_version != NULL) {
		if (write_version(target, git_version)) {
			free(git_version);
			return EXIT_FAILURE;
		}
		free(git_version);
	} else {
		if (write_version(target, version)) {
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
