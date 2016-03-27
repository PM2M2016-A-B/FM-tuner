# ----------------------------------------------------------------------- #
# Copyright (C) 2015-2016 ABHAMON Ronan                                   #
#                                                                         #
# This program is free software: you can redistribute it and/or modify    #
# it under the terms of the GNU General Public License as published by    #
# the Free Software Foundation, either version 3 of the License, or       #
# (at your option) any later version.                                     #
#                                                                         #
# This program is distributed in the hope that it will be useful,         #
# but WITHOUT ANY WARRANTY; without even the implied warranty of          #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
# GNU General Public License for more details.                            #
#                                                                         #
# You should have received a copy of the GNU General Public License       #
# along with this program.  If not, see <http://www.gnu.org/licenses/>.   #
#                                                                         #
# ----------------------------------------------------------------------- #

DEBUG = yes

SRC_DIR = src
INC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SUB_DIRS = hw net utils

CXX = gcc
CXXFLAGS = -Wall -Wextra -pedantic -std=c99 -O0 -D_XOPEN_SOURCE=700 -DEUROPE_VERSION
LDFLAGS = -lm -pthread

ifeq ($(DEBUG), yes)
	CXXFLAGS += -g -DDEBUG
	LDFLAGS +=
else
	CXXFLAGS += -DNDEBUG -s
	LDFLAGS +=
endif

# ----------------------------------------------------------------------- #

SRC_DIRS = $(SRC_DIR) $(foreach dir, $(SUB_DIRS), $(addprefix $(SRC_DIR)/, $(dir)))
INC_DIRS = $(INC_DIR) $(foreach dir, $(SUB_DIRS), $(addprefix $(INC_DIR)/, $(dir)))
OBJ_DIRS = $(OBJ_DIR) $(foreach dir, $(SUB_DIRS), $(addprefix $(OBJ_DIR)/, $(dir)))

# Sources & Headers & Bin

SRC = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
INC = $(foreach dir, $(INC_DIRS), $(addprefix -I, $(dir)))
OBJ = $(addsuffix .o, $(basename $(subst $(SRC_DIR), $(OBJ_DIR), $(SRC))))
BIN = fmtuner

# Make

.PHONY: clean mrproper depend
.SUFFIXES:

all: depend $(BIN)

depend:
	@echo "Creating a list of dependencies..."
	@$(CXX) -MM $(INC) $(SRC) 2> /dev/null | \
	  sed 's/\(.*\.o:\) $(SRC_DIR)\/\(.*\/\)\?\(.*\.c\)/$(OBJ_DIR)\/\2\1 $(SRC_DIR)\/\2\3/' > .depend
	@mkdir -p $(OBJ_DIRS) $(SRC_DIRS) $(INC_DIRS) $(BIN_DIR)

-include .depend

$(BIN): $(OBJ)
	@$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(BIN) $(OBJ) $(LDFLAGS)
	@echo "Done!"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CXXFLAGS) -I$(INC_DIR)/$(subst $(SRC_DIR)/,,$(dir $<)) -c $< -o $@

install: $(BIN)
	@echo "Installation..."
	mv $(BIN_DIR)/$(BIN) /bin/ && mv fmtuner.service /lib/systemd/system/

clean:
	@rm -f $(OBJ)
	$(foreach dir, $(SRC_DIRS), @rm -rf $(dir)/*~ $(dir)/*# $(dir)/*~ $(dir)/*# *~ *#)
	$(foreach dir, $(INC_DIRS), @rm -rf $(dir)/*~ $(dir)/*# $(dir)/*~ $(dir)/*# *~ *#)

mrproper: clean
	@rm -rf $(BIN_DIR)/$(BIN)

rebuild: mrproper all
