PIO = pio
UPLOAD = -t upload

.PHONY: trator build monitor copia limpa

trator:
	@echo "Robô Selecionado: TRATOR"
	$(PIO) run -e trator $(UPLOAD)

build:
	$(PIO) run -e trator

monitor:
	@echo "Monitor Serial (Ctrl+C para sair)"
	$(PIO) device monitor -b 115200

copia:
	@echo "Criando novo repomix"
	npx repomix

limpa:
	rm -rf .pio/
