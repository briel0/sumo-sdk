PIO = pio
UPLOAD = -t upload

.PHONY: all caipora smoker arruela clean monitor sensores

caipora:
	@echo "Robô Selecionado: CAIPORA"
	$(PIO) run -e caipora $(UPLOAD)

smoker:
	@echo "Robô Selecionado: SMOKER"
	$(PIO) run -e smoker $(UPLOAD)

arruela:
	@echo "Robô Selecionado: ARRUELA"
	$(PIO) run -e arruela $(UPLOAD)

sensores:
	@echo "Teste de Sensores: CAIPORA (bancada)"
	$(PIO) run -e caipora_sensores $(UPLOAD) && $(PIO) device monitor -b 115200

monitor:
	@echo "Monitor Serial (Ctrl+C para sair)"
	$(PIO) device monitor -b 115200

copia:
	@echo "Criando novo repomix"
	npx repomix

limpa:
	rm -rf .pio/