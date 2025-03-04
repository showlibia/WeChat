PROTO_DIR = Common/proto
PROTO_FILEE = $(PROTO_DIR)/message.proto
SERVERS := GateServer VerifyServer StatusServer ChatServer1 ChatServer2
PORT = 12345 50052 8090 8091 50056 50055
CLEAN := $(SERVERS)

.PHONY: proto start clean

start: proto $(SERVERS)
	@echo "All servers started in background"

proto:
	@echo "Generating gRPC code..."
	@protoc --proto_path=$(PROTO_DIR) --grpc_out=$(PROTO_DIR) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $(PROTO_FILEE)
	@echo "Generating C++ code..."
	@protoc --proto_path=$(PROTO_DIR) --cpp_out=$(PROTO_DIR) $(PROTO_FILEE)
	@echo "Done."

.PHONY: $(SERVERS)
$(SERVERS):
	@echo "Starting $@"
	@$(MAKE) -C $@ run > $@.log 2>&1 & \
	 echo $$! > $@.pid
	@echo "$@ PID: $$(cat $@.pid)"

.PHONY: logs
logs:
	@for server in $(SERVERS); do \
		if [ -f $$server.log ]; then \
			LOG_FILES="$$LOG_FILES $$server.log"; \
		fi; \
		if [ -f $$server/build/$$server.log ]; then \
			LOG_FILES="$$LOG_FILES $$server/build/$$server.log"; \
		fi; \
		done; \
		if [ -z "$$LOG_FILES" ]; then \
			echo "No log files found"; \
		else \
			echo "Tailing log files: $$LOG_FILES"; \
			grc tail -f $$LOG_FILES;\
		fi

.PHONY: stop
stop:
	@# 1. 终止通过 .pid 文件记录的进程
	@for server in $(SERVERS); do \
		if [ -f $$server.pid ]; then \
			PID=$$(cat $$server.pid); \
			if ps -p $$PID > /dev/null; then \
				kill -9 $$PID 2>/dev/null && echo "[PID] Force stopped $$server (PID: $$PID)" || echo "[PID] Failed to stop $$server (PID: $$PID)"; \
			else \
				echo "[PID] $$server (PID: $$PID) is not running"; \
			fi; \
			rm -f $$server.pid; \
		else \
			echo "[PID] $$server is not running (no .pid file)"; \
		fi; \
	done
    
	@# 2. 释放端口并打印进程信息
	@for port in $(PORT); do \
		PIDS=$$(sudo lsof -ti :$$port); \
		if [ -n "$$PIDS" ]; then \
			echo "[PORT] Port $$port is used by:"; \
			for pid in $$PIDS; do \
				CMD=$$(ps -p $$pid -o comm= 2>/dev/null || echo "unknown"); \
				echo "  - PID $$pid ($$CMD)"; \
			done; \
			echo "[PORT] Killing processes on port $$port"; \
			sudo kill -9 $$PIDS 2>/dev/null || true; \
		else \
			echo "[PORT] Port $$port is already free"; \
		fi; \
	done


rmlog: stop
	@rm -rf $(SERVERS:%=%.log)

clean:
	@for dir in $(CLEAN); do \
		$(MAKE) -C $$dir clean; \
	done