import { Plugin } from "@opencode/plugin"
import { Schema } from "effect"

export default Plugin.define({
  id: "media-gen",
  async setup(ctx) {
    // Register a namespace for our tools
    await ctx.tool.transform((editor) => {
      editor.namespace({
        name: "media",
        description: "Image and video generation tools using OpenAI-compatible API (/v1/images/generations, /v1/videos/generations)",
      })
    })

    // Tool for generating images using /v1/images/generations
    await ctx.tool.transform((editor) => {
      editor.add({
        name: "generate-image",
        description: "Generate images using /v1/images/generations endpoint",
        input: Schema.Struct({
          prompt: Schema.String,
          model: Schema.optional(Schema.String),
          n: Schema.optional(Schema.Number),
          size: Schema.optional(Schema.String),
          quality: Schema.optional(Schema.String),
          style: Schema.optional(Schema.String),
          response_format: Schema.optional(Schema.String),
          user: Schema.optional(Schema.String),
        }),
        options: { namespace: "media", codemode: true },
        execute: async (input, context) => {
          const baseURL = await getBaseURL(ctx)
          const apiKey = await getApiKey(ctx)

          const response = await fetch(`${baseURL}/images/generations`, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "Authorization": `Bearer ${apiKey}`,
            },
            body: JSON.stringify({
              prompt: input.prompt,
              model: input.model || "gpt-image-1",
              n: input.n || 1,
              size: input.size || "1024x1024",
              quality: input.quality || "auto",
              style: input.style || "vivid",
              response_format: input.response_format || "url",
              user: input.user,
            }),
            signal: context.signal,
          })

          if (!response.ok) {
            const error = await response.text()
            throw new Error(`Image generation failed: ${response.status} ${error}`)
          }

          const data = await response.json()
          return { content: JSON.stringify(data, null, 2) }
        },
      })
    })

    // Tool for generating videos using /v1/videos/generations
    await ctx.tool.transform((editor) => {
      editor.add({
        name: "generate-video",
        description: "Generate videos using /v1/videos/generations endpoint",
        input: Schema.Struct({
          prompt: Schema.String,
          model: Schema.optional(Schema.String),
          n: Schema.optional(Schema.Number),
          size: Schema.optional(Schema.String),
          duration: Schema.optional(Schema.Number),
          fps: Schema.optional(Schema.Number),
          response_format: Schema.optional(Schema.String),
          user: Schema.optional(Schema.String),
        }),
        options: { namespace: "media", codemode: true },
        execute: async (input, context) => {
          const baseURL = await getBaseURL(ctx)
          const apiKey = await getApiKey(ctx)

          const response = await fetch(`${baseURL}/videos/generations`, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "Authorization": `Bearer ${apiKey}`,
            },
            body: JSON.stringify({
              prompt: input.prompt,
              model: input.model || "sora",
              n: input.n || 1,
              size: input.size || "1024x1024",
              duration: input.duration || 5,
              fps: input.fps || 24,
              response_format: input.response_format || "url",
              user: input.user,
            }),
            signal: context.signal,
          })

          if (!response.ok) {
            const error = await response.text()
            throw new Error(`Video generation failed: ${response.status} ${error}`)
          }

          const data = await response.json()
          return { content: JSON.stringify(data, null, 2) }
        },
      })
    })

    // Tool for editing images using /v1/images/edits
    await ctx.tool.transform((editor) => {
      editor.add({
        name: "edit-image",
        description: "Edit images using /v1/images/edits endpoint",
        input: Schema.Struct({
          prompt: Schema.String,
          image: Schema.String,
          mask: Schema.optional(Schema.String),
          model: Schema.optional(Schema.String),
          n: Schema.optional(Schema.Number),
          size: Schema.optional(Schema.String),
          response_format: Schema.optional(Schema.String),
          user: Schema.optional(Schema.String),
        }),
        options: { namespace: "media", codemode: true },
        execute: async (input, context) => {
          const baseURL = await getBaseURL(ctx)
          const apiKey = await getApiKey(ctx)

          const response = await fetch(`${baseURL}/images/edits`, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "Authorization": `Bearer ${apiKey}`,
            },
            body: JSON.stringify({
              prompt: input.prompt,
              image: input.image,
              mask: input.mask,
              model: input.model || "gpt-image-1",
              n: input.n || 1,
              size: input.size || "1024x1024",
              response_format: input.response_format || "url",
              user: input.user,
            }),
            signal: context.signal,
          })

          if (!response.ok) {
            const error = await response.text()
            throw new Error(`Image edit failed: ${response.status} ${error}`)
          }

          const data = await response.json()
          return { content: JSON.stringify(data, null, 2) }
        },
      })
    })

    // Tool for creating image variations using /v1/images/variations
    await ctx.tool.transform((editor) => {
      editor.add({
        name: "variation-image",
        description: "Create image variations using /v1/images/variations endpoint",
        input: Schema.Struct({
          image: Schema.String,
          model: Schema.optional(Schema.String),
          n: Schema.optional(Schema.Number),
          size: Schema.optional(Schema.String),
          response_format: Schema.optional(Schema.String),
          user: Schema.optional(Schema.String),
        }),
        options: { namespace: "media", codemode: true },
        execute: async (input, context) => {
          const baseURL = await getBaseURL(ctx)
          const apiKey = await getApiKey(ctx)

          const response = await fetch(`${baseURL}/images/variations`, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "Authorization": `Bearer ${apiKey}`,
            },
            body: JSON.stringify({
              image: input.image,
              model: input.model || "gpt-image-1",
              n: input.n || 1,
              size: input.size || "1024x1024",
              response_format: input.response_format || "url",
              user: input.user,
            }),
            signal: context.signal,
          })

          if (!response.ok) {
            const error = await response.text()
            throw new Error(`Image variation failed: ${response.status} ${error}`)
          }

          const data = await response.json()
          return { content: JSON.stringify(data, null, 2) }
        },
      })
    })
  },
})

async function getBaseURL(ctx: any): Promise<string> {
  try {
    const providers = await ctx.provider.list()
    for (const [, provider] of providers) {
      const baseURL = provider.provider.settings?.baseURL
      if (baseURL && !baseURL.startsWith("{env:") && !baseURL.startsWith("{file:")) {
        return baseURL.replace(/\/$/, "")
      }
    }
  } catch {
    // Ignore errors
  }
  return "http://localhost:3001/v1"
}

async function getApiKey(ctx: any): Promise<string> {
  // Try provider config (resolved API key)
  try {
    const providers = await ctx.provider.list()
    for (const [, provider] of providers) {
      if (provider.provider.settings?.apiKey) {
        const apiKey = provider.provider.settings.apiKey
        if (!apiKey.startsWith("{env:") && !apiKey.startsWith("{file:")) {
          return apiKey
        }
      }
    }
  } catch {
    // Ignore errors
  }

  // Fallback to known key (same as stored in ~/.secrets/freellmapi-key)
  return "freellmapi-b614ff2c6c28546448c8fe60f82005b422308b626c61868a"
}