import os
import glob

# Resolve o diretório raiz do projeto a partir da localização do script
PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def build_webui(source_dir, output_file):
    source_dir = os.path.join(PROJECT_DIR, source_dir)
    output_file = os.path.join(PROJECT_DIR, output_file)
    print(f"Generating {output_file} from {source_dir}...")
    
    # Read CSS
    css_content = ""
    css_path = os.path.join(source_dir, "style.css")
    if os.path.exists(css_path):
        with open(css_path, "r", encoding="utf-8") as f:
            css_content = f.read()
            
    # Read JS
    js_content = ""
    js_path = os.path.join(source_dir, "app.js")
    if os.path.exists(js_path):
        with open(js_path, "r", encoding="utf-8") as f:
            js_content = f.read()
            
    with open(output_file, 'w', encoding='utf-8') as f_out:
        f_out.write("// ARQUIVO GERADO AUTOMATICAMENTE. NÃO EDITE DIRETAMENTE.\n")
        f_out.write("// Modifique os arquivos .html, .css e .js na pasta ui/ e recompile o projeto.\n")
        f_out.write("#pragma once\n")
        f_out.write("#include <Arduino.h>\n\n")
        
        for filepath in glob.glob(os.path.join(source_dir, '*.html')):
            filename = os.path.basename(filepath)
            name, ext = os.path.splitext(filename)
            var_name = f"{name.upper().replace('-', '_')}_HTML"
            
            with open(filepath, 'r', encoding='utf-8') as f_in:
                html_content = f_in.read()
                
            # Bundler: Injeta CSS e JS substituindo as tags reais para garantir 1 único arquivo no ESP32
            html_content = html_content.replace('<link rel="stylesheet" href="style.css">', f"<style>\n{css_content}\n</style>")
            html_content = html_content.replace('<script src="app.js"></script>', f"<script>\n{js_content}\n</script>")
                
            f_out.write(f"const char {var_name}[] PROGMEM = R\"rawliteral(\n")
            f_out.write(html_content)
            f_out.write("\n)rawliteral\";\n\n")
            print(f" - Embutido {filename} como {var_name} (CSS e JS injetados)")

if __name__ == "__main__":
    # Execução manual (teste)
    build_webui("ui", "include/services/WebUI.hpp")
else:
    # Execução via PlatformIO (extra_script)
    try:
        Import("env")
        env.Execute(lambda *args, **kwargs: build_webui("ui", "include/services/WebUI.hpp"))
    except NameError:
        pass
