# Overclocked — SQL in Plain English Engine ⚡

> **HackFest 2.0 (Track 1, Problem #4)**  
> A high-performance, hybrid local LLM + deterministic rule-engine fallback application built with **C++17**, **Qt6**, and **PostgreSQL**.

---

## 🚀 Architecture Overview

Overclocked bridges the gap between natural human language and structured database queries with a zero-latency fallback loop:
1. **The AI Layer:** Attempts to translate natural language prompts via a local **Ollama** sidecar running `phi3:mini` (port `11434`).
2. **The Deterministic Safety Net:** If the local model times out or stumbles, the engine instantly seamless-routes the query through an external, hot-reloadable **`rules.json`** configuration engine.
3. **The Presentation Layer:** A custom-styled dark-mode **Qt6** dashboard featuring an instant data grid and a collapsible, transparent SQL inspection panel.

---

## 🛠️ Tech Stack
* **Language:** C++17
* **GUI Framework:** Qt6 (`QNetworkAccessManager`, `QTableWidget`)
* **Database & Connector:** PostgreSQL (`hackathon_db`), `libpqxx`
* **Local LLM Sidecar:** Ollama (`phi3:mini`)
* **Config Engine:** Dynamic JSON rule mapper (`rules.json`)

---

## ⚙️ Setup & Installation

### 1. Database Configuration
Ensure PostgreSQL is running and seed your tables (`students`, `server_logs`):
```bash
cat << 'EOF' > README.md
# Overclocked — SQL in Plain English Engine ⚡

> **HackFest 2.0 (Track 1, Problem #4)**  
> A high-performance, hybrid local LLM + deterministic rule-engine fallback application built with **C++17**, **Qt6**, and **PostgreSQL**.

---

## 🚀 Architecture Overview

Overclocked bridges the gap between natural human language and structured database queries with a zero-latency fallback loop:
1. **The AI Layer:** Attempts to translate natural language prompts via a local **Ollama** sidecar running `phi3:mini` (port `11434`).
2. **The Deterministic Safety Net:** If the local model times out or stumbles, the engine instantly seamless-routes the query through an external, hot-reloadable **`rules.json`** configuration engine.
3. **The Presentation Layer:** A custom-styled dark-mode **Qt6** dashboard featuring an instant data grid and a collapsible, transparent SQL inspection panel.

---

## 🛠️ Tech Stack
* **Language:** C++17
* **GUI Framework:** Qt6 (`QNetworkAccessManager`, `QTableWidget`)
* **Database & Connector:** PostgreSQL (`hackathon_db`), `libpqxx`
* **Local LLM Sidecar:** Ollama (`phi3:mini`)
* **Config Engine:** Dynamic JSON rule mapper (`rules.json`)

---

## ⚙️ Setup & Installation

### 1. Database Configuration
Ensure PostgreSQL is running and seed your tables (`students`, `server_logs`):
sudo service postgresql start
2. Start Local LLM Sidecar
Make sure Ollama is active with the required model:

Bash
ollama serve
# In another terminal tab:
ollama pull phi3:mini
3. Build & Run
Bash
mkdir build && cd build
cmake ..
make -j$(nproc)
cp ../rules.json .
./TeamOverclocked
