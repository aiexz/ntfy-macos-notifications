package main

import (
	"bufio"
	"encoding/json"
	"go.uber.org/zap"
	"gopkg.in/yaml.v3"
	"log"
	"net/http"
	"ntfy-macos-notifications/banner"
	"os"
	"strings"
)

type Config struct {
	Topics []struct {
		Url string `yaml:"url"`
	} `yaml:"topics"`
}

func main() {
	logger, err := zap.NewProduction()
	defer logger.Sync()

	undo := zap.ReplaceGlobals(logger)
	defer undo()

	f, err := os.Open("config.yaml")
	if err != nil {
		zap.L().Sugar().Fatal(err)
	}
	defer f.Close()

	var c Config
	if err := yaml.NewDecoder(f).Decode(&c); err != nil {
		zap.L().Sugar().Fatal(err)
	}
	for _, topic := range c.Topics {
		if strings.HasSuffix(topic.Url, "/json") {
			go NTFYProcessor(topic.Url)
		} else {
			go NTFYProcessor(topic.Url + "/json")
		}
	}
	select {}
}

type NTFYMessage struct {
	Id       string   `json:"id"`
	Time     int      `json:"time"`
	Event    string   `json:"event"`
	Topic    string   `json:"topic"`
	Title    string   `json:"title"`
	Message  string   `json:"message"`
	Priority int      `json:"priority"`
	Tags     []string `json:"tags"`
}

func NTFYProcessor(link string) {
	resp, err := http.Get(link)
	if err != nil {
		log.Fatal(err)
	}
	defer resp.Body.Close()
	scanner := bufio.NewScanner(resp.Body)
	for scanner.Scan() {
		zap.L().Sugar().Debug("New message", scanner.Text())
		var n NTFYMessage
		if err := json.Unmarshal(scanner.Bytes(), &n); err != nil {
			zap.L().Sugar().Error(err)
		}
		if n.Event == "message" {
			noti := banner.Notification{
				Title:           n.Title,
				Subtitle:        n.Topic,
				InformativeText: n.Message,
			}
			if n.Priority == 5 {
				noti.Urgent = true
			}
			noti.Send()
		}
	}
}
