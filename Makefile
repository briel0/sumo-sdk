PIO = pio
UPLOAD = -t upload

.PHONY: all caipora tupan clean monitor

caipora:
	@echo "Robô Selecionado: CAIPORA"
	$(PIO) run -e caipora $(UPLOAD)

smoker:
	@echo "Robô Selecionado: SMOKER"
	$(PIO) run -e smoker $(UPLOAD)

monitor:
	@echo "Monitor Serial (Ctrl+C para sair)"
	$(PIO) device monitor -b 115200