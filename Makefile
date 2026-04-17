CC = cc
CFLAGS = -O2 -Wall -Wextra -std=c11
LDFLAGS = -lm

.PHONY: all clean test test-c test-py test-prompt

all: klaus

klaus: klaus.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

tests/test_klaus: tests/test_klaus.c
	$(CC) $(CFLAGS) -w -o $@ $< $(LDFLAGS)

test: test-c test-py test-prompt
	@echo ""
	@echo "=== Integration tests ==="
	./klaus --prompt "I am terrified and alone"
	@echo ""
	./klaus --prompt "мне страшно и одиноко"
	@echo ""
	./klaus --prompt "je suis furieux et triste"
	@echo ""
	./klaus --prompt "אני מפחד ובודד"
	@echo ""
	@echo "=== ALL TESTS PASSED ==="

test-c: tests/test_klaus
	@tests/test_klaus

test-py:
	@python3 tests/test_klaus.py

test-prompt:
	@tests/test_prompt_multiline.sh

clean:
	rm -f klaus tests/test_klaus
