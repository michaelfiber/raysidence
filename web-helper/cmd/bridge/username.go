package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"

	"github.com/pkg/errors"
)

type getUsernameBody struct {
	DeviceType string `json:"devicetype"`
}

type Username struct {
	Error struct {
		ErrorType   int64  `json:"type"`
		Address     string `json:"address"`
		Description string `json:"description"`
	} `json:"error"`

	Success struct {
		Username string `json:"username"`
	} `json:"success"`
}

var (
	LinkButtonNotPressed = errors.New("link button not pressed")
)

func GetUsername(server string, deviceType string) (string, error) {
	body := getUsernameBody{
		DeviceType: deviceType,
	}

	bodyJson, err := json.Marshal(body)
	if err != nil {
		return "", errors.Wrap(err, "json.Marshal(body)")
	}

	reader := bytes.NewReader(bodyJson)

	resp, err := http.Post(fmt.Sprintf("http://%s/api", server), "application/json", reader)
	if err != nil {
		return "", errors.Wrap(err, "http.Post")
	}
	defer resp.Body.Close()

	response, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", errors.Wrap(err, "io.ReadAll")
	}

	var usernames []Username
	if err := json.Unmarshal(response, &usernames); err != nil {
		return "", errors.Wrap(err, "json.Unmarshal")
	}

	if len(usernames) == 0 {
		return "", errors.New("empty response")
	}

	if usernames[0].Error.ErrorType == 101 {
		return "", LinkButtonNotPressed
	}

	if len(usernames[0].Error.Description) > 0 {
		return "", errors.New(usernames[0].Error.Description)
	}

	if len(usernames[0].Success.Username) == 0 {
		return "", errors.New("no username")
	}

	return usernames[0].Success.Username, nil
}
