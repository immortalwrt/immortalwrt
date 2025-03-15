# (c) 2019-2021 Thomas Bernard
# For GNU Make

# CI_COMMIT_TAG / CI_COMMIT_BRANCH / CI_COMMIT_SHORT_SHA are gitlab-ci
# predefined variables
# see https://docs.gitlab.com/ee/ci/variables/predefined_variables.html
ifneq ($(CI_COMMIT_TAG),)
GITREF = $(CI_COMMIT_TAG)
else
ifneq ($(CI_COMMIT_BRANCH),)
GITREF = $(CI_COMMIT_BRANCH)-$(CI_COMMIT_SHORT_SHA)
else
ISGITREPO := $(shell git rev-parse --is-inside-work-tree)
ifeq ($(ISGITREPO),true)
# <tag> or <branch>-<short commit ref>
GITREF := $(shell git describe --exact-match --tags 2> /dev/null || echo "`git rev-parse --abbrev-ref HEAD`-`git rev-parse --short HEAD`" )
endif
endif
endif

ifneq ($(GITREF),)
CPPFLAGS += -DMINIUPNPD_GIT_REF=\"$(GITREF)\"
endif
